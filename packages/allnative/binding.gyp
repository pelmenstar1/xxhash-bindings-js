{
"targets": [
  {
    "target_name": "xxhash-allnative",
    "sources": [ 
      "../../native/index.cpp",
      "../../native/fileHash.cpp",
      "../../native/directoryHash.cpp",
      "../../native/directoryHashAsync.cpp",
      "../../native/directoryHashUtils.cpp",
      "../../native/oneshotHash.cpp",
      "../../native/createHashState.cpp",

      "../../native/xxhash.c",
      "../../native/v8Utils.cpp",
      "../../native/v8HashState.cpp",
      "../../native/v8ObjectParser.cpp",
      "../../native/fileHashWorker.cpp",
     
      "../../native/platform/blockReader.cpp",
      "../../native/platform/fullPathBuilder.cpp",
      "../../native/platform/memoryMap.cpp",
      "../../native/platform/platformError.cpp",
      "../../native/platform/directoryIterator.cpp",
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ],
    'conditions': [
      ['OS=="win"', { 
        'defines': [ '_HAS_EXCEPTIONS=1' ], 
         'msvs_settings': {
            'VCCLCompilerTool': { 'AdditionalOptions': ['/EHsc'], },
        },
      }],
      ['OS=="mac"', {
        'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
        }
      }]
    ],
    "configurations": {
      "Debug": {
        "cflags": ['-fexceptions', '-O0'],
        "cflags_cc": ['-fexceptions', '-O0'],
      },
      "Release": {
        "cflags": ['-fexceptions', '-O2'],
        "cflags_cc": ['-fexceptions', '-O2'],
      }
    }
  }
]
}