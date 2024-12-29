{
"targets": [
  {
    "target_name": "xxhash-allnative",
    "sources": [ 
      "../../native/index.cpp",
      "../../native/fileHash.cpp",
      "../../native/directoryHash.cpp",
      "../../native/oneshotHash.cpp",
      "../../native/createHashState.cpp",

      "../../native/xxhash.c",
      "../../native/v8Utils.cpp",
      "../../native/v8HashState.cpp",
      "../../native/v8ObjectParser.cpp",
      "../../native/fileHashWorker.cpp",
     
      "../../native/platform/blockReader.cpp",
      "../../native/platform/memoryMap.cpp",
      "../../native/platform/platformError.cpp",
      "../../native/platform/directoryIterator.cpp",
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")"
    ],
    "cflags": ['-fexceptions', '-O2'],
    "cflags_cc": ['-fexceptions', '-O2'],
    'conditions': [
      ['OS=="win"', { 
        'defines': [ '_HAS_EXCEPTIONS=1' ], 
         'msvs_settings': {
            'VCCLCompilerTool': { 'AdditionalOptions': ['/EHsc'], },
        },
      }]
    ]
  }
]
}