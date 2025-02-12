import { setupTests } from '../base/file';
import { expectToThrowAsyncFactory } from '../base/helpers';
import lib from 'xxhash-bindings';

setupTests({
  getFileFactory: (name) => lib[name].fileAsync,
  expectToThrowError: expectToThrowAsyncFactory(),
});
