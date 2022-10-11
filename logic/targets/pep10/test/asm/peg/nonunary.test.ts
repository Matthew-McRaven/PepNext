import { asm } from '../../../src/lib';

describe('asm.peg for nonunary', () => {
  it('detects nonunary instructions at start of line with no addr mode', () => {
    const root = asm.peg.parse('hi 5\n');
    // C[0] is default section, C[0].C[0] is first line of code
    const nonunary = root.C[0].C[0];
    // Check that the nonunary op is correct
    expect(nonunary.A.op).toBe('hi');
    expect(nonunary.type).toBe('nonunary');
    expect(nonunary.A.arg).toBe(5);
    expect(nonunary.A.addr).toBe(null);
  });
  it('detects nonunary instructions with no addr mode with comments', () => {
    const root = asm.peg.parse('hi 5;world\n');
    // C[0] is default section, C[0].C[0] is first line of code
    const nonunary = root.C[0].C[0];
    // Check that the nonunary op is correct
    expect(nonunary.A.op).toBe('hi');
    expect(nonunary.type).toBe('nonunary');
    expect(nonunary.A.arg).toBe(5);
    expect(nonunary.A.addr).toBe(null);
    expect(nonunary.A.comment).toBe('world');
  });

  it('detects nonunary instructions at start of line with addr mode', () => {
    const root = asm.peg.parse('hi 5,v\n');
    // C[0] is default section, C[0].C[0] is first line of code
    const nonunary = root.C[0].C[0];
    // Check that the nonunary op is correct
    expect(nonunary.A.op).toBe('hi');
    expect(nonunary.type).toBe('nonunary');
    expect(nonunary.A.arg).toBe(5);
    expect(nonunary.A.addr).toBe('v');
  });
  it('detects nonunary instructions with addr mode with comments', () => {
    const root = asm.peg.parse('hi 5,v;world\n');
    // C[0] is default section, C[0].C[0] is first line of code
    const nonunary = root.C[0].C[0];
    // Check that the nonunary op is correct
    expect(nonunary.A.op).toBe('hi');
    expect(nonunary.type).toBe('nonunary');
    expect(nonunary.A.arg).toBe(5);
    expect(nonunary.A.addr).toBe('v');
    expect(nonunary.A.comment).toBe('world');
  });
});
