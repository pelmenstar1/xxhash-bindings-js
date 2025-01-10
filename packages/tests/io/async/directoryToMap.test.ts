import { setupTests } from '../base/directory';
import { expectToThrowAsyncFactory } from '../base/helpers';

setupTests({
  getDirectoryFactory: (lib, name) => lib[name].directoryToMapAsync,
  expectToThrowError: expectToThrowAsyncFactory(),
});
