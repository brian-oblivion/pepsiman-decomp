# asm-differ configuration.
#
#   .venv/bin/python3 tools/asm-differ/diff.py <func>
#
# Use this to READ a diff -- it gives side-by-side mnemonics with register and
# branch-target tracking. tools/funcdiff.py answers the different question of
# "does it match yet", in words, and is the one to script against.
def apply(config, args):
    config["baseimg"] = "disk/SLPS_017.62"
    config["myimg"] = "build/SLPS_017.62"
    config["mapfile"] = "build/pepsiman.map"
    config["source_directories"] = ["src", "include"]
    config["arch"] = "mipsel"
    config["objdump_executable"] = "tools/binutils/bin/mipsel-linux-gnu-objdump"
    config["makeflags"] = []
    config["expected_dir"] = "expected/"
