import { beforeAll } from 'vitest';
import { setupDirectories, setupTests } from './directoryTestUtils';

beforeAll(setupDirectories);
setupTests((lib, name) => lib[name].directoryToMap);
