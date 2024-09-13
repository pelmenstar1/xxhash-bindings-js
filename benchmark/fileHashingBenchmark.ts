import { FileHashingType, xxhash3, xxhash32 } from "..";
import { generateRandomFileContent } from "./randomDataGenerator";
import { benchmark, runAll } from "./suite";

const DATA_FILE_NAME = "gb1"
const DATA_FILE_PATH = `./.data/${DATA_FILE_NAME}`

benchmark({
    name: "xxhash3 map",
    worker: () => {
        return xxhash3.file(DATA_FILE_PATH, 1)
    }
})

benchmark({
  name: "xxhash3 block",
  worker: () => {
      return xxhash3.file(DATA_FILE_PATH, 1, FileHashingType.BLOCK)
  }
})

async function main() {
    // 1 GB
    //await generateRandomFileContent(DATA_FILE_NAME, 1024 * 1024 * 1024);

    runAll();
}

void main();
