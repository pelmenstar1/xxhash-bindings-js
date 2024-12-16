export type AnyArray<Arr extends unknown[]> = {
  [Index in keyof Arr]: unknown;
};

export function hideArgumentTypes<Args extends unknown[], R>(
  func: (...args: Args) => R,
): (...args: AnyArray<Args>) => R {
  return (...args) => func(...(args as Args));
}
