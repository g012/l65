const std = @import("std");

const targets: []const std.Target.Query = &.{
    .{ .os_tag = .windows, .cpu_arch = .x86_64, .cpu_model = .baseline },
    .{ .os_tag = .windows, .cpu_arch = .aarch64, .cpu_model = .baseline },
    .{ .os_tag = .macos, .cpu_arch = .x86_64, .cpu_model = .baseline, .os_version_min = std.Target.Query.OsVersion{ .semver = .{ .major = 10, .minor = 7, .patch = 0 } } },
    .{ .os_tag = .macos, .cpu_arch = .aarch64, .cpu_model = .baseline, .os_version_min = std.Target.Query.OsVersion{ .semver = .{ .major = 11, .minor = 0, .patch = 0 } } },
    .{ .os_tag = .linux, .cpu_arch = .x86_64, .cpu_model = .baseline, .abi = .musl },
    .{ .os_tag = .linux, .cpu_arch = .aarch64, .cpu_model = .baseline },
};

fn addCExecutable(b: *std.Build, name: []const u8, target: anytype, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    return b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
}

fn setupExe(exe: *std.Build.Step.Compile, main_file: []const u8, embed_output: *const std.Build.LazyPath) !void {
    exe.addCSourceFiles(.{
        .files = &.{ "lfs.c", "lpeg.c", main_file },
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

    const embed_exe = addCExecutable(b, "embed", target, optimize);
    embed_exe.addCSourceFile(.{ .file = b.path("embed.c"), .flags = &.{"-std=c99"} });
    embed_exe.linkLibC();

    const embed = b.addRunArtifact(embed_exe);
    embed.addArg("-o");
    const embed_output = embed.addOutputFileArg("scripts.h");
    embed.addFileArg(b.path("l65cfg.lua"));
    embed.addFileArg(b.path("asm.lua"));
    embed.addFileArg(b.path("6502.lua"));
    embed.addFileArg(b.path("dkjson.lua"));
    embed.addFileArg(b.path("l65.lua"));
    embed.addFileArg(b.path("re.lua"));
    embed.addFileArg(b.path("nes.l65"));
    embed.addFileArg(b.path("pce.l65"));
    embed.addFileArg(b.path("vcs.l65"));

    const embed_7801 = b.addRunArtifact(embed_exe);
    embed_7801.addArg("-o");
    const embed_7801_output = embed_7801.addOutputFileArg("scripts_7801.h");
    embed_7801.addFileArg(b.path("asm.lua"));
    embed_7801.addFileArg(b.path("uPD7801.lua"));
    embed_7801.addFileArg(b.path("dkjson.lua"));
    embed_7801.addFileArg(b.path("l7801.lua"));
    embed_7801.addFileArg(b.path("l65cfg.lua"));
    embed_7801.addFileArg(b.path("re.lua"));
    embed_7801.addFileArg(b.path("scv.l7801"));

    ///////////////////////////////////////////////////////////////////////////
    // Build for current machine

    var exe = addCExecutable(b, "l65", target, optimize);

    exe.step.dependOn(&embed.step);
    try setupExe(exe, "main.c", &embed_output);
    b.installArtifact(exe);

    var exe_7801 = addCExecutable(b, "l7801", target, optimize);

    exe_7801.step.dependOn(&embed_7801.step);
    try setupExe(exe_7801, "l7801.c", &embed_7801_output);
    b.installArtifact(exe_7801);

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
        const rel_exe = addCExecutable(b, "l65", b.resolveTargetQuery(t), .ReleaseSmall);
        const target_output = b.addInstallArtifact(rel_exe, .{
            .dest_dir = .{
                .override = .{
                    .custom = try t.zigTriple(b.allocator),
                },
            },
        });
        rel_exe.step.dependOn(&embed.step);
        try setupExe(rel_exe, "main.c", &embed_output);
        release_step.dependOn(&target_output.step);

        const rel_exe_7801 = addCExecutable(b, "l7801", b.resolveTargetQuery(t), .ReleaseSmall);
        const target_output_7801 = b.addInstallArtifact(rel_exe_7801, .{
            .dest_dir = .{
                .override = .{
                    .custom = try t.zigTriple(b.allocator),
                },
            },
        });
        rel_exe_7801.step.dependOn(&embed_7801.step);
        try setupExe(rel_exe_7801, "l7801.c", &embed_7801_output);
        release_step.dependOn(&target_output_7801.step);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Build samples as test
    // TODO
}
