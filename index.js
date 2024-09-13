import bindings from "bindings";

const addon = bindings("xxhash");

export const FileHashingType = {
  MAP: 0,
  BLOCK: 1
}

export const xxhash32 = { 
  oneshot: addon.xxhash32,
  file: addon.xxhash32_file
}

export const xxhash64 = { 
  oneshot: addon.xxhash64,
  file: addon.xxhash64_file
}

export const xxhash3 = { 
  oneshot: addon.xxhash3,
  file: addon.xxhash3_file
}

export const xxhash3_128 = { 
  oneshot: addon.xxhash3_128,
  file: addon.xxhash3_128_file
}