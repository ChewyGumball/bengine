ShaderLibraryInfo = provider(
    fields = {
        "include_directories": "directories where the library files reside",
    },
)

def _bengine_shader_library_impl(ctx):
    library_directory = "{}_shader_includes".format(ctx.label.name)

    isolated_files = []
    for src in ctx.files.srcs:
        isolated_file = ctx.actions.declare_file("{}/{}".format(library_directory, src.short_path))
        ctx.actions.symlink(
            isolated_file,
            target_file = src,
            progress_message = "Symlinking {} to {}".format(src.short_path, isolated_file.short_path),
        )

        isolated_files.add(isolated_file)

    dependency_library_depsets = []
    dependency_file_depsets = []
    for dep in ctx.attr.deps:
        dependency_file_depsets.add(dep[DefaultInfo].files)
        dependency_library_depsets.add(dep[ShaderLibraryInfo].include_directories)

    return [
        DefaultInfo(files = depset(direct = [isolated_files], transitive = dependency_file_depsets)),
        ShaderLibraryInfo(include_directories = depset(direct = [library_directory], transitive = dependency_library_depsets)),
    ]

bengine_shader_library = rule(
    implementation = _bengine_shader_library_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = True),
        "deps": attr.label_list(providers = [DefaultInfo, ShaderLibraryInfo]),
    },
)

def _bengine_shader_impl(ctx):
    vertex_file = ctx.file.vertex_src
    fragment_file = ctx.file.fragment_src
    semantics_file = ctx.file.semantics_file

    if ctx.attr.output_prefix_path != "":
        output_file_name = "{}/{}.shader".format(ctx.attr.output_prefix_path, ctx.label.name)
    else:
        output_file_name = "{}.shader".format(ctx.label.name)

    output_file = ctx.actions.declare_file(output_file_name)

    include_directories = []
    for dep in ctx.attr.deps:
        include_directories.add(dep[ShaderLibraryInfo].include_directories)

    args = ctx.actions.args()
    args.add("--vertex-source", vertex_file)
    args.add("--fragment-source", fragment_file)
    args.add("--semantics-file", semantics_file)
    args.add_joined("--include-directories", include_directories, join_with = ",")
    args.add("--output", output_file)
    args.add("--quiet")

    ctx.actions.run(
        mnemonic = "ShaderCompiler",
        executable = ctx.executable._compiler,
        arguments = [args],
        progress_message = "{}: compiling {}".format(ctx.label.name, output_file_name),
        inputs = [vertex_file, fragment_file, semantics_file],
        outputs = [output_file],
    )

    return [
        DefaultInfo(
            files = depset([output_file]),
            runfiles = ctx.runfiles(files = [output_file]),
        ),
    ]

bengine_shader = rule(
    implementation = _bengine_shader_impl,
    attrs = {
        "vertex_src": attr.label(allow_single_file = True),
        "fragment_src": attr.label(allow_single_file = True),
        "semantics_file": attr.label(allow_single_file = True),
        "deps": attr.label_list(providers = [ShaderLibraryInfo]),
        "output_prefix_path": attr.string(),
        "_compiler": attr.label(
            default = Label("//assets/compilers/shader:shader_compiler"),
            allow_single_file = True,
            executable = True,
            cfg = "exec",
        ),
    },
)
