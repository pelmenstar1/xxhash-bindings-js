type Row = string[]

type Table = {
  header: Row,
  content: Row[],

  toString(): string,
  print(): void
}

type TableBuilder = {
  append(row: Row): void;

  create(): Table;
}

function computeColumnWidth(header: Row, content: Row[], columnIndex: number): number {
  let width = header.length;

  for (let i = 0; i < content.length; i++) {
    width = Math.max(width, content[i][columnIndex].length);
  }

  return width;
}

function displayRow(row: Row, widths: number[]): string {
  let result = "";

  for (let i = 0; i < row.length; i++) {
    const entry = row[i];
    const width = widths[i];

    let padded = entry.padEnd(width, " ");
    padded += " | ";

    result += padded;
  }

  return result;
}

function computeTotalTableWidth(widths: number[]) {
  let result = 0;

  for (const width of widths) {
    result += width + 3;
  }

  return result;
}

function createTable(header: Row, content: Row[]): Table {
  const table = {
    header,
    content,

    toString() {
      const widths: number[] = [];
      for (let i = 0; i < header.length; i++) {
        widths.push(computeColumnWidth(header, content, i));
      }

      let result = displayRow(header, widths) + '\n';
      result += '-'.repeat(computeTotalTableWidth(widths)) + '\n';
      result += content.map(row => displayRow(row, widths)).join('\n');

      return result;
    },
    print() {
      console.log(table.toString());
    }
  };

  return table;
}

export function tableBuilder(header: Row): TableBuilder {
  const rows: Row[] = [];

  return {
    append(row) {
      if (row.length != header.length) {
        throw new Error("Row length should be the same as the header length");
      }
      
      rows.push(row);
    },

    create() {
      return createTable(header, rows);
    }
  }
}

