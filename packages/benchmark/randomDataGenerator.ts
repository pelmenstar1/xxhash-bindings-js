import fs from 'fs';
import path from 'path';
import crypto from 'node:crypto';

export function generateRandomFileContent(filePath: string, size: number) {
  const BUFFER_SIZE = 4096;

  let handle: number | undefined;

  fs.mkdirSync(path.dirname(filePath), { recursive: true });

  try {
    handle = fs.openSync(filePath, 'w+');

    const buffer = Buffer.allocUnsafe(BUFFER_SIZE);
    let offset = 0;

    while (offset < size) {
      const bytesToWrite = Math.min(BUFFER_SIZE, size - offset);
      crypto.randomFillSync(buffer, 0, bytesToWrite);

      fs.writeSync(handle, buffer, 0, bytesToWrite, offset);
      offset += BUFFER_SIZE;
    }
  } finally {
    fs.closeSync(handle);
  }
}
