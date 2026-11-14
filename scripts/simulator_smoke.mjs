import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { resolve, dirname } from 'node:path';
import { once } from 'node:events';
import assert from 'node:assert/strict';
import { mkdtempSync, cpSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const firmware = resolve(root, 'firmware');
const binary = resolve(firmware, '.pio/build/simulator/program' + (process.platform === 'win32' ? '.exe' : ''));
// A private commissioned fixture leaves the user's simulator setup untouched.
const fixture = mkdtempSync(resolve(tmpdir(), 'pitclaw-smoke-'));
cpSync(resolve(firmware, 'data'), resolve(fixture, 'data'), { recursive: true });
writeFileSync(resolve(fixture, '.sim_setup_complete'), '1');
const port = 18937;
const child = spawn(binary, ['--port', String(port)], {
  cwd: fixture, windowsHide: true,
  env: { ...process.env, SDL_VIDEODRIVER: 'dummy', SDL_AUDIODRIVER: 'dummy' },
  stdio: ['ignore', 'pipe', 'pipe'],
});
let output = '';
child.stdout.on('data', b => { output = (output + b).slice(-10000); });
child.stderr.on('data', b => { output = (output + b).slice(-10000); });
child.on('error', e => { output += e.message; });
let ws;
const messages = [];
const pause = ms => new Promise(r => setTimeout(r, ms));
async function waitFor(predicate, label) {
  for (let i = 0; i < 100; i++) {
    const value = predicate();
    if (value) return value;
    if (child.exitCode !== null) throw Error('Simulator exited: ' + output);
    await pause(100);
  }
  throw Error('Timeout: ' + label + '\nMessages: ' + JSON.stringify(messages.slice(-3)) + '\n' + output);
}
async function command(cmd, predicate) {
  messages.length = 0;
  ws.send(JSON.stringify(cmd));
  return waitFor(() => messages.find(predicate), JSON.stringify(cmd));
}
try {
  let response;
  for (let i = 0; i < 100; i++) {
    try { response = await fetch(`http://127.0.0.1:${port}/`); break; }
    catch { await pause(100); }
  }
  assert.equal(response?.status, 200, output);
  assert.match(await response.text(), /Pit Claw/i);
  for (const asset of ['app.js', 'style.css', 'manifest.json', 'sw.js']) {
    assert.equal((await fetch(`http://127.0.0.1:${port}/${asset}`)).status, 200);
  }
  ws = new WebSocket(`ws://127.0.0.1:${port}/ws`);
  ws.addEventListener('message', event => messages.push(JSON.parse(event.data)));
  await Promise.race([once(ws, 'open'), pause(10000).then(() => { throw Error('WebSocket open timeout'); })]);
  await command({type: 'set', sp: 250}, m => m.type === 'data' && m.sp === 250);
  await command({type: 'alarm', meat1Target: 203}, m => m.type === 'data' && m.meat1Target === 203);
  await command({type: 'alarm', pitBand: 15}, m => m.type === 'data' && m.meat1Target === 203);
  await command({type: 'set', sp: 999}, m => m.type === 'data' && m.sp === 250);
  await command({type: 'session', action: 'download', format: 'csv'}, m =>
    m.type === 'session' && m.action === 'download' && m.data.startsWith('timestamp,'));
  await command({type: 'session', action: 'new'}, m => m.type === 'session' && m.action === 'reset');
  console.log('PASS: SDL startup, HTTP assets, WebSocket setpoint/alarms/validation/export/reset');
} finally {
  ws?.close();
  child.kill();
  await Promise.race([once(child, 'exit'), pause(2000)]);
  rmSync(fixture, { recursive: true, force: true });
}
