# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| Latest release | Yes |
| Older releases | No |

## Reporting a Vulnerability

If you discover a security vulnerability, please report it responsibly:

1. **Do not** open a public issue
2. Email the maintainer or use [GitHub's private vulnerability reporting](https://github.com/MrMatt57/pitclaw/security/advisories/new)
3. Include steps to reproduce and potential impact
4. Allow reasonable time for a fix before public disclosure

## Scope

This is a hobbyist IoT device for BBQ temperature control. It runs on a local Wi-Fi network and does not handle sensitive personal data. Security concerns are primarily:

- Web UI served over HTTP (local network only)
- WebSocket protocol (unauthenticated, local network only)
- OTA firmware updates (local network only)
