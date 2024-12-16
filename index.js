import bindings from "bindings";

const addon = bindings("xxhash");

function modifiedFileHasher(hasher) {
  return (options) => {
    return hasher(options.path, options.seed, options.type, options.offset, options.length);
  }
}

function xxHashVariant(oneshot, file, createState) {
  return {
    oneshot,
    createState,
    file: modifiedFileHasher(file),
  }
}

export const FileHashingType = {
  MAP: 0,
  BLOCK: 1
}

export const xxhash32 = xxHashVariant(addon.xxhash32, addon.xxhash32_file, addon.xxhash32_createState);
export const xxhash64 = xxHashVariant(addon.xxhash64, addon.xxhash64_file, addon.xxhash64_createState);
export const xxhash3 = xxHashVariant(addon.xxhash3, addon.xxhash3_file, addon.xxhash3_createState);
export const xxhash3_128 = xxHashVariant(addon.xxhash3_128, addon.xxhash3_128_file, addon.xxhash3_128_createState);