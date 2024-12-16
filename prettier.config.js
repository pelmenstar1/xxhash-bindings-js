/**
 * @type {import("prettier").Config}
 */
const config = {
  singleQuote: true,
  overrides: [
    {
      files: '.editorconfig',
      options: { parser: 'yaml' },
    },
  ],
};

export default config;
