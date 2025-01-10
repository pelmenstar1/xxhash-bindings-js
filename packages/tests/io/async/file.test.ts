import { setupTests } from '../base/file';
import { expectToThrowAsyncFactory } from '../base/helpers';

setupTests({
  getFileFactory: (lib, name) => lib[name].fileAsync,
  expectToThrowError: expectToThrowAsyncFactory(),
});
