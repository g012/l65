const std = @import("std");

const major: u32 = 1;
const minor: u32 = 3;
const revision: u32 = 0;

const targets: []const std.Target.Query = &.{
    .{ .os_tag = .windows, .cpu_arch = .x86_64, .cpu_model = .baseline },
    .{ .os_tag = .windows, .cpu_arch = .aarch64, .cpu_model = .baseline },
    .{ .os_tag = .macos, .cpu_arch = .x86_64, .cpu_model = .baseline, .os_version_min = std.zig.CrossTarget.OsVersion{ .semver = .{ .major = 10, .minor = 7, .patch = 0 } } },
    .{ .os_tag = .macos, .cpu_arch = .aarch64, .cpu_model = .baseline, .os_version_min = std.zig.CrossTarget.OsVersion{ .semver = .{ .major = 11, .minor = 0, .patch = 0 } } },
    .{ .os_tag = .linux, .cpu_arch = .x86_64, .cpu_model = .baseline, .abi = .musl },
    .{ .os_tag = .linux, .cpu_arch = .aarch64, .cpu_model = .baseline },
};

fn setupExe(exe: *std.Build.Step.Compile, embed_output: *const std.Build.LazyPath) !void {
    exe.addCSourceFiles(.{
        .files = &.{ "lfs.c", "lpeg.c", "main.c" },
        .flags = &.{},
    });

    exe.addIncludePath(embed_output.dirname());

    exe.linkLibC();
}

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    ///////////////////////////////////////////////////////////////////////////
    // Embed tool

    const version_storage = struct { var str: [32]u8 = undefined; };
    const version = try std.fmt.bufPrint(version_storage.str[0..], "{d}.{d}.{d}", .{ major, minor, revision });
    const l65cfg_lua_in = b.addConfigHeader(
        .{
            .style = .{ .cmake = .{ .path = "l65cfg.lua.in" } },
            .include_path = "l65cfg.lua",
        },
        .{
            .L65_VERSION_MAJOR = major,
            .L65_VERSION_MINOR = minor,
            .L65_VERSION_REVISION = revision,
            .L65_VERSION = version,
        },
    );
    // FIXME current version (0.12-dev) of ConfigHeader adds a C comment on top of file
    //    - comment "try output.appendSlice(c_generated_line);" in ConfigHeader.zig to avoid it
    

    const embed_exe = b.addExecutable(.{
        .name = "embed",
        .target = target,
        .optimize = optimize,
    });
    embed_exe.addCSourceFile(.{ .file = .{ .path = "embed.c" }, .flags = &.{ "-std=c99" } });
    embed_exe.linkLibC();

    const embed = b.addRunArtifact(embed_exe);
    embed.step.dependOn(&l65cfg_lua_in.step);
    embed.addArg("-o");
    const embed_output = embed.addOutputFileArg("scripts.h");
    embed.addFileArg(l65cfg_lua_in.getOutput());
    embed.addFileArg(.{ .path = "asm.lua" });
    embed.addFileArg(.{ .path = "6502.lua" });
    embed.addFileArg(.{ .path = "dkjson.lua" });
    embed.addFileArg(.{ .path = "l65.lua" });
    embed.addFileArg(.{ .path = "re.lua" });
    embed.addFileArg(.{ .path = "nes.l65" });
    embed.addFileArg(.{ .path = "pce.l65" });
    embed.addFileArg(.{ .path = "vcs.l65" });

    ///////////////////////////////////////////////////////////////////////////
    // Build for current machine

    var exe = b.addExecutable(.{
        .name = "l65",
        .target = target,
        .optimize = optimize,
    });

    exe.step.dependOn(&embed.step);
    try setupExe(exe, &embed_output);
    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run l65");
    run_step.dependOn(&run_cmd.step);

    ///////////////////////////////////////////////////////////////////////////
    // Release binaries

    const release_step = b.step("release", "Generate a release");
    for (targets) |t| {
        const rel_exe = b.addExecutable(.{
            .name = "l65",
            .target = b.resolveTargetQuery(t),
            .optimize = .ReleaseSmall,
        });
        const target_output = b.addInstallArtifact(rel_exe, .{
            .dest_dir = .{
                .override = .{
                    .custom = try t.zigTriple(b.allocator),
                },
            },
        });
        rel_exe.step.dependOn(&embed.step);
        try setupExe(rel_exe, &embed_output);
        release_step.dependOn(&target_output.step);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Build samples as test
    // TODO
}
