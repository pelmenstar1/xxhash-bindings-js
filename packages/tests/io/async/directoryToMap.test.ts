import { setupTests } from '../base/directory';
import { expectToThrowAsyncFactory } from '../base/helpers';
import lib from 'xxhash-bindings';

setupTests({
  getDirectoryFactory: (name) => lib[name].directoryToMapAsync,
  expectToThrowError: expectToThrowAsyncFactory(),
});
