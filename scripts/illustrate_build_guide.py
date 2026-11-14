from pathlib import Path
from html import escape

OUT = Path(__file__).resolve().parents[1] / 'docs/images/build-guide'
OUT.mkdir(parents=True, exist_ok=True)
INK='#173248'; MUTED='#536777'; BLUE='#176baf'; ORANGE='#ba4c16'; GREEN='#15775f'
class SVG:
    def __init__(self, title, sub, height=760):
        self.h=height
        self.a=[f'<svg xmlns="http://www.w3.org/2000/svg" width="1120" height="{height}" viewBox="0 0 1120 {height}" role="img" aria-labelledby="title desc"><title id="title">{escape(title)}</title><desc id="desc">{escape(sub)}</desc><defs><marker id="arrow" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto-start-reverse"><path d="M0 0 L8 4 L0 8" fill="#536777"/></marker></defs><rect width="1120" height="{height}" fill="#f4f7fa"/>']
        self.text(36,45,title,28,INK,True); self.text(36,76,sub,16,MUTED)
    def text(self,x,y,t,size=18,color=INK,bold=False):
        self.a.append(f'<text x="{x}" y="{y}" font-family="Arial, sans-serif" font-size="{size}" fill="{color}" font-weight="{700 if bold else 400}">{escape(t)}</text>')
    def box(self,x,y,w,h,title,lines=(),color=INK):
        self.a.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="12" fill="white" stroke="{color}" stroke-width="2"/>')
        self.text(x+18,y+32,title,20,color,True)
        for i,line in enumerate(lines): self.text(x+18,y+61+i*25,line,17)
    def path(self,coords,color=MUTED,arrow=False,dash=False):
        d='M'+' L'.join(f'{x} {y}' for x,y in coords)
        self.a.append(f'<path d="{d}" fill="none" stroke="{color}" stroke-width="3" stroke-linejoin="round"'+(' marker-end="url(#arrow)"' if arrow else '')+(' stroke-dasharray="7 5"' if dash else '')+'/>')
    def dot(self,x,y,color=INK): self.a.append(f'<circle cx="{x}" cy="{y}" r="5" fill="{color}"/>')
    def ground(self,x,y):
        self.path([(x,y),(x,y+16)],INK)
        for i,w in enumerate([34,22,10]): self.path([(x-w/2,y+16+i*7),(x+w/2,y+16+i*7)],INK)
    def footer(self,t): self.text(36,self.h-24,t,15,MUTED)
    def save(self,name): (OUT/name).write_text('\n'.join(self.a)+ '</svg>\n',encoding='utf-8')

s=SVG('01 / Power and signal map','Functional connections — not a physical cable view. All grounds connect electrically.',740)
s.box(40,125,220,110,'12V barrel input',['Verify jack polarity','Power off for wiring'],ORANGE)
s.box(390,125,290,110,'MP1584EN buck',['Adjust to 5.00V unloaded','Then verify under load'],ORANGE)
s.path([(260,160),(390,160)],ORANGE,True);s.text(295,147,'12V',17,ORANGE,True)
s.box(820,125,260,110,'5015 fan section',['12V load supply','See diagram 03'],ORANGE)
s.path([(315,160),(315,105),(950,105),(950,125)],ORANGE,True);s.dot(315,160,ORANGE)
s.box(390,310,290,145,'WT32 display',['EXT 1 = 5V; EXT 2 = GND','DEBUG 2 / verified 3V3','supplies ADC + pull-ups'],BLUE)
s.path([(530,235),(530,310)],ORANGE,True);s.text(545,280,'5V',17,ORANGE,True)
s.box(820,310,260,145,'MG90S servo',['5V from power rail','Return to power GND','GPIO13 = signal'],BLUE)
s.path([(680,200),(755,200),(755,335),(820,335)],ORANGE,True);s.text(709,185,'5V',17,ORANGE,True)
s.box(40,310,245,145,'ADS1115 + probes',['VDD + dividers = 3.3V','SDA = GPIO10','SCL = GPIO11'],GREEN)
s.path([(390,355),(285,355)],GREEN,True);s.text(303,340,'3.3V',17,GREEN,True)
s.path([(285,405),(390,405)],BLUE);s.text(305,430,'I2C',17,BLUE,True)
s.box(40,515,1040,135,'Shared ground, separate return routes',[
'Buck GND, display GND, ADS1115 GND, probe sleeves, MOSFET source and servo GND connect.',
'Route fan and servo current back to the power ground; keep it out of probe-ground leads.',
'3.3V is NOT on the eight-pin extension connector. Verify each conductor with a meter.'],INK)
s.footer('Colors identify nets in this drawing; they do not specify the colors of purchased cables.')
s.save('01-system-map.svg')

s=SVG('02 / Build one probe channel, then repeat','Three identical circuits: Pit → A0, Meat 1 → A1, Meat 2 → A2. A3 stays unused.',800)
s.text(75,140,'3.3V',20,GREEN,True)
s.path([(160,132),(160,175)],GREEN)
s.a.append('<rect x="148" y="175" width="24" height="70" fill="white" stroke="#173248" stroke-width="3"/>')
s.text(200,211,'10kΩ / 1%',20,INK,True)
s.path([(160,245),(160,290),(515,290)],GREEN);s.dot(160,290,GREEN)
s.text(210,276,'SENSE NODE',17,GREEN,True)
s.box(515,245,235,90,'ADS1115',['A0, A1 or A2'],GREEN)
s.path([(160,290),(160,410)],GREEN);s.dot(160,340,GREEN)
s.box(65,410,240,100,'2.5mm mono jack',['Tip = sense node','Sleeve = GND'],INK)
s.path([(160,510),(160,545)],INK);s.ground(160,545)
s.text(53,602,'Probe thermistor connects',16,MUTED);s.text(53,625,'between tip and sleeve.',16,MUTED)
s.path([(160,340),(380,340),(380,420)],GREEN);s.dot(380,340,GREEN)
s.path([(355,420),(405,420)],INK);s.path([(355,432),(405,432)],INK)
s.path([(380,432),(380,545)],INK);s.ground(380,545)
s.text(417,425,'100nF',19,INK,True);s.text(417,451,'node → GND',16,MUTED)
s.box(805,115,275,495,'WT32 EXT net list',[
'1    5V from buck',
'2    GND',
'3    GPIO10 → SDA',
'4    GPIO11 → SCL',
'5    GPIO12 → fan gate',
'6    GPIO13 → servo',
'7    GPIO14 → piezo',
'8    GPIO21: unused',
'',
'Pin numbers only.',
'Confirm pin 1 on the PCB.',
'Cable views can reverse',
'the apparent left/right order.'],BLUE)
s.box(515,365,235,165,'ADC power',['VDD = 3.3V','GND + ADDR = GND','Verify 3.3V I2C pull-ups','and local bypass cap.'],GREEN)
s.box(40,660,1040,85,'Meter check before a probe',['10kΩ across tip/sleeve: ~1.650V, 13200 counts, 184.2°F at a measured 3.300V rail.'],GREEN)
s.footer('Identify jack contacts with a plugged-in connector and continuity meter. Leave switched contacts unused.')
s.save('02-probe-and-harness.svg')

s=SVG('03 / Existing output wiring','Retained hardware — the direct 3.3V IRLZ44N gate drive remains UNQUALIFIED.',810)
s.text(95,145,'+12V',21,ORANGE,True)
s.path([(140,155),(140,195)],ORANGE)
s.box(60,195,175,105,'5015 blower',['+  to 12V','−  to drain'],ORANGE)
s.path([(140,300),(140,350),(400,350),(400,385)],ORANGE)
s.box(315,385,235,125,'IRLZ44N',['G = gate; D = drain','S = source; tab = drain','Verify actual part pinout.'],INK)
s.text(410,374,'D',18,ORANGE,True)
s.path([(400,510),(400,565)],INK);s.ground(400,565);s.text(418,545,'S',18,INK,True)
s.path([(65,450),(315,450)],BLUE);s.text(57,430,'GPIO12',20,BLUE,True);s.text(288,437,'G',18,BLUE,True)
s.dot(190,450,BLUE);s.path([(190,450),(190,480)],BLUE)
s.a.append('<rect x="178" y="480" width="24" height="55" fill="white" stroke="#173248" stroke-width="3"/>')
s.text(80,514,'10kΩ',19,INK,True)
s.path([(190,535),(190,565),(400,565)],INK);s.dot(400,565,INK)
s.box(655,135,425,165,'MG90S servo',[
'Power → 5V distribution (not 12V)',
'Ground → power GND',
'Signal → GPIO13',
'Remove linkage before endpoint tests.'],BLUE)
s.box(655,330,425,135,'Passive piezo',[
'Qualified GPIO-compatible piezo only.',
'Drive → GPIO14; return → GND.',
'Confirm type and electrical load.'],BLUE)
s.box(655,495,425,110,'Keep cables identifiable',[
'Fan pair and servo cable: label both ends.',
'RJ45 powered pinout is not yet defined.'],INK)
s.box(40,650,1040,105,'Qualification gate — not an approved production driver',[
'Confirm the fan’s supply-PWM specification, reliable restart, current and MOSFET heating.',
'This drawing adds no gate driver or protection parts. See the production review before release.'],ORANGE)
s.footer('A software “closed” command does not prove mechanical closure or a closed damper after power loss.')
s.save('03-output-wiring.svg')

s=SVG('04 / Assemble in stages; close the case last','Bench-test each subassembly before installing it. Use one matching CAD/STL revision.',850)
steps=[('1  LABEL', ['Map display cable pins.','Pre-wire panel jacks.','Label both lead ends.']),('2  BUILD', ['Repeat sensor circuit ×3.','Build power/output section.','Insulate and secure parts.']),('3  TEST', ['Set buck to 5.00V first.','Test each input/output.','Then test combined loads.']),('4  FIT', ['Dry-fit boards and ports.','Route and secure leads.','Fit strain relief; close case.'])]
for i,(title,lines) in enumerate(steps):
    x=35+i*275;s.box(x,120,245,150,title,lines,BLUE)
    if i<3:s.path([(x+245,190),(x+270,190)],MUTED,True)
s.box(40,325,500,300,'USB flashing / probe checks',[
'12V supply disconnected; fan/servo unplugged.',
'Isolate the WT32 external 5V lead.',
'Isolate output signals to unpowered loads/drivers.',
'Connect the display’s built-in USB.',
'',
'Flash bench → status → resistor/open/short tests.',
'Disconnect USB before restoring external wiring.'],BLUE)
s.box(580,325,500,300,'External-power motor checks',[
'Built-in USB stays disconnected.',
'Restore checked external 5V/output wiring.',
'Use a 3.3V-logic UART adapter:',
'Adapter RX ← DEBUG 3 (TX)',
'Adapter TX → DEBUG 4 (RX)',
'Adapter GND → DEBUG 7; VCC unconnected.',
'Power unit first, then connect adapter to PC.',
'Commands: fan 100 / servo 1500 / beep / off'],ORANGE)
s.box(40,670,1040,125,'Final checks before a supervised cook',[
'Pit unplug + short • ADC loss • reset/reboot • Wi-Fi loss • combined motor/display loads',
'Record rail voltages, temperature readings, fan behavior and calibrated damper endpoints.',
'Bench acceptance is not production approval; the review lists the remaining release gates.'],GREEN)
s.footer('Current main: bezel + rear shell + kickstand. Local inset-box prototypes require their own matching fit checks.')
s.save('04-assembly-and-test.svg')
print('Generated four build-guide SVGs in',OUT)
