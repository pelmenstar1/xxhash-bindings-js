import fs from "fs";
import crypto from "node:crypto";

export async function generateRandomFileContent(filePath: string, size: number): Promise<string> {
    const BUFFER_SIZE = 4096;
  
    let handle: fs.promises.FileHandle | undefined;
  
    try {
      handle = await fs.promises.open(filePath, "w+");
  
      const buffer = Buffer.allocUnsafe(BUFFER_SIZE);
      let offset = 0;
  
      while (offset < size) {
        const bytesToWrite = Math.min(BUFFER_SIZE, size - offset);
        crypto.randomFillSync(buffer, 0, bytesToWrite);
  
        fs.writeSync(handle.fd, buffer, 0, bytesToWrite, offset);
        offset += BUFFER_SIZE;
      }
  
      return filePath;
    } finally {
      await handle?.close();
    }
  }