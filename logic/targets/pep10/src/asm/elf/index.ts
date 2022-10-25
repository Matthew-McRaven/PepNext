/* eslint-disable camelcase */
import { DefintionState, ISymbolNative, TraversalPolicy } from '@pepnext/logic-symbol';
import {
  Writer, IWriter, ELFSymbol32, st_info,
} from '@pepnext/logic-elf';
import { Root, SectionGroup, TypedNode } from '../ast/nodes';
import { treeToHex, updateSymbolShndx } from '../visitors';

export const createElf = (node:TypedNode):unknown => {
  const writer:IWriter = new Writer(32);
  const sectionGroups = node.C as SectionGroup[];

  // Write global info
  writer.writeEType(1n/* ET_REL: https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-43405/index.html */);
  writer.writeEMachine(0x5041n /* PA */); // P 10 with the second digit in hex.
  writer.writeOSABI(0n /* ELFOSABI_NONE */);

  // Write code / data sections.

  const collater = new Map<string, Uint8Array[]>();
  // Gather all sections that have the same name.
  sectionGroups.forEach((s) => {
    let array: Uint8Array[] = [];
    if (collater.has(s.A.name)) array = collater.get(s.A.name) || [];
    array.push(treeToHex(s));
    collater.set(s.A.name, array);
  });

  // Merge bytes of all sections that have the same name.
  const collated = new Map<string, Uint8Array>();
  Array.from(collater.entries()).forEach(([k, v]) => {
    // Allocate an array to hold concat of all arrays.
    const length = v.reduce((prevLen:number, arr:Uint8Array) => prevLen + arr.length, 0);
    const combinedBytes = new Uint8Array(length);
    // Insert each array at the next open spot in the combined array.
    v.reduce((prevLen, arr:Uint8Array) => {
      combinedBytes.set(arr, prevLen);
      return prevLen + arr.length;
    }, 0);
    // Then store the final section:byte mapping.
    collated.set(k, combinedBytes);
  });

  // Helper to update the st_shndx of each section
  const setShndx = (name:string, st_shndx:bigint) => {
    node.walk((child:TypedNode) => {
      if (child.T !== 'sectionGroup') return;
      if (child.A.name !== name) return;
      // eslint-disable-next-line no-param-reassign
      child.A.st_shndx = st_shndx;
    });
  };

  // Create sections for each set of bytes, and update all relevant SectionGroups.
  collated.forEach((v, k) => {
    const shndx = writer.writeSectionBytes(k, {} as any, v);
    setShndx(k, shndx);
  });

  // Update shndx on symbol declarations
  updateSymbolShndx(node);

  // Write symbol tables
  const rootTab = (node as Root).A.symtab;
  const symbols = rootTab.enumerateSymbols(TraversalPolicy.wholeTree) as Array<ISymbolNative>;
  // The only symbols that matter for linking are globals and undefined
  const exportedSymbols = symbols.filter((s) => (s.definitionState() === DefintionState.undefined || s.binding() === 'global'));
  // Undefined symbols may show up in multiple tables, but we only want one entry in the ELF's symtab.
  const deduplicatedSymbols = new Map<string, ISymbolNative>();
  exportedSymbols.forEach((s) => {
    if (s.definitionState() === DefintionState.undefined && !deduplicatedSymbols.has(s.name())) deduplicatedSymbols.set(s.name(), s);
    else if (s.binding() === 'global') deduplicatedSymbols.set(s.name(), s);
  });
  // Create the ELF symbol spec for the deduplicate symbols.
  const elfSymbols: ELFSymbol32[] = [];
  deduplicatedSymbols.forEach((s) => {
    elfSymbols.push({
      size: 32,
      st_name: s.name(),
      st_size: BigInt(s.size()),
      st_value: BigInt(s.value()),
      st_shndx: s.sectionIndex(),
      st_info: st_info(s.binding(), s.type()),
      st_other: 0n, // Only contains visibility, which we will leave at default for now
    } as ELFSymbol32);
  });
  // writer.writeSymbols('.symtab', elfSymbols);

  // Write relocation entries
  // Make sure to relocate any usages of pushed-down symbols.
  // TODO: Implement after writing visitor.

  // Write addr:line mapping
  // TODO: Implement after writing visitor.

  // Write MMIO fields as JSON, add to OS.MMIO.
  // If none, do not write section at all.
  // TODO: Implement when OS is assembling successfully

  // Write debug info
  // TODO: Implement after adding debug info

  return node as unknown;
};
