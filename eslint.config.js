import globals from 'globals';
import pluginJs from '@eslint/js';
import tseslint from 'typescript-eslint';
import eslintConfigPrettier from 'eslint-plugin-prettier/recommended';

/** @type {import('eslint').Linter.Config[]}  */
export default [
  {
    ignores: ['.yarn/*', 'dist/*', '.pnp.cjs', '.pnp.loader.mjs'],
  },
  { languageOptions: { globals: globals.node } },
  { ...pluginJs.configs.recommended, ignores: ['**/*.{ts,cts}'] },
  eslintConfigPrettier,
  ...tseslint.configs.strict.map((config) => ({
    ...config,
    rules: {
      '@typescript-eslint/no-require-imports': 'off',
      '@typescript-eslint/no-explicit-any': 'off',
    },
  })),
];
