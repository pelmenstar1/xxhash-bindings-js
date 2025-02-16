{
"targets": [
  {
    "target_name": "xxhash",
    "sources": [ 
      "../../native/index.cpp",
      "../../native/fileHash.cpp",
      "../../native/oneshotHash.cpp",
      "../../native/createHashState.cpp",

      "../../native/xxhash.c",
      "../../native/jsHashState.cpp",
      "../../native/jsObjectParser.cpp",
      "../../native/fileHashWorker.cpp",
     
      "../../native/platform/blockReader.cpp",
      "../../native/platform/memoryMap.cpp",
      "../../native/platform/platformError.cpp",
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")",
       "<!(node -p \"require('node-addon-api').include_dir\")"
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