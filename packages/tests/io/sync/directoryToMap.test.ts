import { setupTests } from '../base/directory';
import { expectToThrowSyncFactory, syncToAsync } from '../base/helpers';
import lib from 'xxhash-bindings';

setupTests({
  getDirectoryFactory: (name) => syncToAsync(lib[name].directoryToMap),
  expectToThrowError: expectToThrowSyncFactory(),
});
