{
"targets": [
  {
    "target_name": "xxhash",
    "sources": [ 
      "native/index.cpp",
      "native/fileHashing.cpp",
      "native/hashers.cpp",
      "native/oneshots.cpp",
      "native/xxhash.c",
      "native/errorMacro.cpp",
      "native/v8Utils.cpp",
      "native/v8HashAdapter.cpp"
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ],
    "conditions": [
      ['OS == "win"', {
        "sources": [
          "native/platform/windows/memoryMap.cpp",
          "native/platform/windows/platformOperationStatus.cpp",
          "native/platform/windows/blockReader.cpp",
        ]
      }],
      ['OS == "linux"', {
        "sources": [
          "native/platform/linux/memoryMap.cpp",
          "native/platform/linux/platformOperationStatus.cpp",
          "native/platform/linux/blockReader.cpp",
        ]
      }]
    ]
  }
]
}