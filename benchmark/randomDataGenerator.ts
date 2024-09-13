import fs from "fs";
import crypto from "node:crypto";

const DATA_PATH = "./.data/"

export async function generateRandomFileContent(fileName: string, size: number): Promise<string> {
    const BUFFER_SIZE = 4096;
  
    await fs.promises.mkdir(DATA_PATH);
  
    const path = `${DATA_PATH}/${fileName}`;
    let handle: fs.promises.FileHandle | undefined;
  
    try {
      handle = await fs.promises.open(path, "w+");
  
      const buffer = Buffer.allocUnsafe(BUFFER_SIZE);
      let offset = 0;
  
      while (offset < size) {
        const bytesToWrite = Math.min(BUFFER_SIZE, size - offset);
        crypto.randomFillSync(buffer, 0, bytesToWrite);
  
        fs.writeSync(handle.fd, buffer, 0, bytesToWrite, offset);
        offset += BUFFER_SIZE;
      }
  
      return path;
    } finally {
      await handle?.close();
    }
  }