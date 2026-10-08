// Ghidra headless post-script: create the functions found by tools/analyze.py, name them,
// and write the decompiler's pseudo-C for every function of this image to <out>/c/<img>.c
// Usage (analyzeHeadless ... -postScript DarklandsExport.java <in_dir> <out_dir>)
//@category Darklands
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.SourceType;
import com.google.gson.*;
import java.io.*;
import java.nio.file.*;

public class DarklandsExport extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        String inDir = args[0], outDir = args[1];
        String img = currentProgram.getName().replace(".bin", "");
        JsonObject spec = JsonParser.parseString(new String(Files.readAllBytes(Paths.get(inDir, img + ".json")))).getAsJsonObject();
        SegmentedAddressSpace sp = (SegmentedAddressSpace) currentProgram.getAddressFactory().getDefaultAddressSpace();
        // labels for code outside this image
        for (JsonElement e : spec.getAsJsonArray("labels")) {
            JsonArray a = e.getAsJsonArray();
            Address ad = sp.getAddress(a.get(0).getAsInt(), a.get(1).getAsInt());
            try { createLabel(ad, a.get(2).getAsString(), true, SourceType.IMPORTED); } catch (Exception ex) { }
            try {
                Function lf = getFunctionAt(ad);
                if (lf == null) { disassemble(ad); lf = createFunction(ad, a.get(2).getAsString()); }
                if (lf != null) lf.setCallingConvention(a.get(3).getAsString());
            } catch (Exception ex) { }
        }
        // DS = DGROUP everywhere (MSC medium model), so data references resolve into DGROUP
        ghidra.program.model.lang.Register ds = currentProgram.getRegister("DS");
        Address lo = sp.getAddress(0, 0), hi = sp.getAddress(0x2000, 0xffff);
        currentProgram.getProgramContext().setValue(ds, lo, hi, java.math.BigInteger.valueOf(0x321a));
        // C strings in the initialised part of DGROUP
        Memory mem = currentProgram.getMemory();
        int run = 0; long start = 0;
        for (long lin = 0x321a0; lin < 0x3a360; lin++) {
            byte b = mem.getByte(sp.getAddress(0x321a, (int)(lin - 0x321a0)));
            if (b >= 0x20 && b < 0x7f) { if (run == 0) start = lin; run++; }
            else {
                if (b == 0 && run >= 4) {
                    try { createAsciiString(sp.getAddress(0x321a, (int)(start - 0x321a0)), run + 1); } catch (Exception ex) { }
                }
                run = 0;
            }
        }
        JsonArray own = spec.getAsJsonArray("own");
        for (JsonElement e : own) {
            JsonArray a = e.getAsJsonArray();
            Address ad = sp.getAddress(a.get(0).getAsInt(), a.get(1).getAsInt());
            disassemble(ad);
        }
        for (JsonElement e : own) {
            JsonArray a = e.getAsJsonArray();
            Address ad = sp.getAddress(a.get(0).getAsInt(), a.get(1).getAsInt());
            Function f = getFunctionAt(ad);
            if (f == null) f = createFunction(ad, a.get(2).getAsString());
            else f.setName(a.get(2).getAsString(), SourceType.IMPORTED);
            if (f != null) f.setCallingConvention(a.get(3).getAsString());
        }
        DecompInterface ifc = new DecompInterface();
        DecompileOptions opt = new DecompileOptions();
        ifc.setOptions(opt);
        ifc.openProgram(currentProgram);
        Files.createDirectories(Paths.get(outDir, "c"));
        int ok = 0, bad = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(Paths.get(outDir, "c", img + ".c").toFile()))) {
            w.println("// " + img + ": Ghidra pseudo-C (machine output; reference only, not compilable)");
            for (JsonElement e : own) {
                JsonArray a = e.getAsJsonArray();
                Address ad = sp.getAddress(a.get(0).getAsInt(), a.get(1).getAsInt());
                Function f = getFunctionAt(ad);
                w.printf("%n// ---- %s  %04x:%04x%n", a.get(2).getAsString(), a.get(0).getAsInt(), a.get(1).getAsInt());
                if (f == null) { w.println("// (no function)"); bad++; continue; }
                DecompileResults r = ifc.decompileFunction(f, 60, monitor);
                if (r != null && r.decompileCompleted()) { w.println(r.getDecompiledFunction().getC()); ok++; }
                else { w.println("// decompile failed: " + (r == null ? "" : r.getErrorMessage())); bad++; }
            }
        }
        println("DarklandsExport " + img + ": decompiled " + ok + ", failed " + bad);
    }
}
