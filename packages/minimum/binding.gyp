{
"targets": [
  {
    "target_name": "xxhash-min",
    "sources": [ 
      "../native/index.cpp",
      "../native/oneshotHash.cpp",
      "../native/createHashState.cpp",

      "../native/hashers.cpp",
      "../native/xxhash.c",
      "../native/errorMacro.cpp",
      "../native/v8Utils.cpp",
      "../native/v8HashAdapter.cpp",
      "../native/v8HashState.cpp"
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ],
    "defines": [
      'XXHASH_BINDINGS_MIN'
    ]
  }
]
}