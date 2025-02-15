import { setupTests } from './fileTestGenerator';
import { expectToThrowSyncFactory, syncToAsync } from './helpers';
import lib from 'xxhash-bindings';

setupTests({
  getFileFactory: (name) => syncToAsync(lib[name].file),
  expectToThrowError: expectToThrowSyncFactory(),
});
