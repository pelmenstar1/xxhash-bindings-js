import { setupTests } from '../base/directory';
import { expectToThrowSyncFactory, syncToAsync } from '../base/helpers';

setupTests({
  getDirectoryFactory: (lib, name) => syncToAsync(lib[name].directoryToMap),
  expectToThrowError: expectToThrowSyncFactory(),
});
