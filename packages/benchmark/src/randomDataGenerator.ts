import { promises as fs } from 'fs';
import path from 'path';
import crypto from 'node:crypto';

export async function generateRandomFileContent(
  filePath: string,
  size: number,
) {
  const BUFFER_SIZE = 4096;

  let handle: fs.FileHandle;

  await fs.mkdir(path.dirname(filePath), { recursive: true });

  try {
    handle = await fs.open(filePath, 'w+');

    const buffer = Buffer.allocUnsafe(BUFFER_SIZE);
    let offset = 0;

    while (offset < size) {
      const bytesToWrite = Math.min(BUFFER_SIZE, size - offset);
      crypto.randomFillSync(buffer, 0, bytesToWrite);

      await fs.appendFile(handle, buffer.subarray(0, bytesToWrite));
      offset += BUFFER_SIZE;
    }
  } finally {
    await handle?.close();
  }
}
