import { FileHashingOptions, FileHashingType, xxhash3 } from "..";
import { generateRandomFileContent } from "./randomDataGenerator";
import { benchmark, runAll } from "./suite";
import fs from "fs";

const DATA_FILE_NAME = "gb1"
const DATA_FILE_PATH = `./.data/${DATA_FILE_NAME}`

const mapOptions: FileHashingOptions<number> = {
  path: DATA_FILE_PATH,
  seed: 1
}

const blockOptions: FileHashingOptions<number> = {
  path: DATA_FILE_PATH,
  seed: 1,
  type: FileHashingType.BLOCK
}

benchmark({
  name: "xxhash3 map",
  worker: () => {
    return xxhash3.file(mapOptions)
  }
})

benchmark({
  name: "xxhash3 block",
  worker: () => {
    return xxhash3.file(blockOptions)
  }
})

async function main() {
  if (!fs.existsSync(DATA_FILE_PATH)) {
    // 1 GB
    await generateRandomFileContent(DATA_FILE_NAME, 1024 * 1024 * 1024);
  }

  runAll();
}

void main();
