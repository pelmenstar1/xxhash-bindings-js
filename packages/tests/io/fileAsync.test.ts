import { setupTests } from './fileTestGenerator';
import { expectToThrowAsyncFactory } from './helpers';
import lib from 'xxhash-bindings';

setupTests({
  getFileFactory: (name) => lib[name].fileAsync,
  expectToThrowError: expectToThrowAsyncFactory(),
});
