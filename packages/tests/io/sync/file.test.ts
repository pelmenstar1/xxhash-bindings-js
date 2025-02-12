import { setupTests } from '../base/file';
import { expectToThrowSyncFactory, syncToAsync } from '../base/helpers';
import lib from 'xxhash-bindings';

setupTests({
  getFileFactory: (name) => syncToAsync(lib[name].file),
  expectToThrowError: expectToThrowSyncFactory(),
});
