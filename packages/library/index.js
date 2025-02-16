import { createRequire } from 'module';

const require = createRequire(import.meta.url);
const addon = require(`./xxhash-${process.platform}-${process.arch}.node`);

function toPromise(func) {
  return (options) => {
    return new Promise((resolve, reject) => {
      func(options, (error, value) => {
        if (error === undefined) {
          resolve(value);
        } else {
          reject(error);
        }
      });
    });
  };
}

function xxHashVariant(name) {
  return {
    oneshot: addon[`${name}_oneshot`],
    createState: addon[`${name}_createState`],
    file: addon[`${name}_file`],
    fileAsync: toPromise(addon[`${name}_fileAsync`]),
  };
}

export const xxhash32 = xxHashVariant('xxhash32');
export const xxhash64 = xxHashVariant('xxhash64');
export const xxhash3 = xxHashVariant('xxhash3');
export const xxhash3_128 = xxHashVariant('xxhash3_128');

export default { xxhash32, xxhash64, xxhash3, xxhash3_128 };
