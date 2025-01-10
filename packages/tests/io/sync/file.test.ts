import { setupTests } from '../base/file';
import { expectToThrowSyncFactory, syncToAsync } from '../base/helpers';

setupTests({
  getFileFactory: (lib, name) => syncToAsync(lib[name].file),
  expectToThrowError: expectToThrowSyncFactory(),
});
