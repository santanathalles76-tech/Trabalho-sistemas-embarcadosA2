// Executa o binário AVR real (o mesmo que roda no Arduino Uno) no emulador avr8js
// e aplica uma sequência de eventos nos pinos, imprimindo a serial.
// Uso (depois de ./compilar_avr.sh e npm install nesta pasta):
//   avr-objcopy -O ihex ../build/avr/a9-painel-senhas.elf a9.hex
//   node rodar_avr.js a9.hex '[[2000,4,0],[2200,4,1],[2500,"LER",9]]'
// Cada evento é [ms, pino, nível]; ["LER", pino] imprime o estado de um pino.
const fs = require('fs');
const a = require('avr8js');
const [hexFile, roteiroJson] = process.argv.slice(2);
const program = new Uint16Array(0x8000 / 2);
const bytes = new Uint8Array(program.buffer);
for (const line of fs.readFileSync(hexFile, 'utf8').split('\n')) {
  if (line[0] !== ':' || line.substr(7, 2) !== '00') continue;
  const n = parseInt(line.substr(1, 2), 16), addr = parseInt(line.substr(3, 4), 16);
  for (let i = 0; i < n; i++) bytes[addr + i] = parseInt(line.substr(9 + i * 2, 2), 16);
}
const cpu = new a.CPU(program);
new a.AVRTimer(cpu, a.timer0Config); new a.AVRTimer(cpu, a.timer1Config); new a.AVRTimer(cpu, a.timer2Config);
const portB = new a.AVRIOPort(cpu, a.portBConfig), portC = new a.AVRIOPort(cpu, a.portCConfig), portD = new a.AVRIOPort(cpu, a.portDConfig);
const usart = new a.AVRUSART(cpu, a.usart0Config, 16e6);
let serial = ''; usart.onByteTransmit = (b) => { serial += String.fromCharCode(b); };
const twi = new a.AVRTWI(cpu, a.twiConfig, 16e6);
let twiBytes = 0;
twi.eventHandler = { start() { twi.completeStart(); }, stop() { twi.completeStop(); }, connectToSlave() { twi.completeConnect(true); },
  writeByte() { twiBytes++; twi.completeWrite(true); }, readByte() { twi.completeRead(0xff); } };
const pino = (n) => (n < 8 ? [portD, n] : [portB, n - 8]);
for (let n = 2; n <= 13; n++) { const [p, b] = pino(n); p.setPin(b, true); }   // botões soltos (pull-up)
const roteiro = JSON.parse(roteiroJson);   // [[ms, pino, nivel]]
const ms = () => cpu.cycles / 16000;
const estado = (n) => { const [p, b] = pino(n); return p.pinState(b) === a.PinState.High ? 1 : 0; };
let i = 0; const fimMs = roteiro[roteiro.length - 1][0] + 500;
while (ms() < fimMs) {
  while (i < roteiro.length && ms() >= roteiro[i][0]) {
    const [t, n, v] = roteiro[i++];
    if (n === 'LER') console.log(`t=${t} ms  pino ${v} = ${estado(v)}`);
    else { const [p, b] = pino(n); p.setPin(b, !!v); }
  }
  a.avrInstruction(cpu); cpu.tick();
}
console.log('--- serial ---\n' + serial.trim());
console.log('--- bytes I2C enviados ao LCD: ' + twiBytes);
