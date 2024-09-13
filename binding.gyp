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
      "native/errorMacro.cpp"
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
          "native/platform/windows/fileUtils.cpp",
        ]
      }],
      ['OS == "linux"', {
        "sources": [
          "native/platform/linux/memoryMap.cpp",
          "native/platform/linux/platformOperationStatus.cpp",
          "native/platform/linux/fileUtils.cpp",
          "native/platform/linux/blockReader.cpp",
        ]
      }]
    ]
  }
]
}