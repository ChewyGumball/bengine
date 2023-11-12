MeshInfo = provider(
    fields = {
        "source_file": "label of the source file",
    },
)

def _bengine_obj_mesh_impl(ctx):
    input_file = ctx.file.src

    if ctx.attr.output_prefix_path != "":
        output_file_name = "{}/{}.mesh".format(ctx.attr.output_prefix_path, ctx.label.name)
    else:
        output_file_name = "{}.mesh".format(ctx.label.name)

    output_file = ctx.actions.declare_file(output_file_name)

    args = ctx.actions.args()
    args.add("--input", input_file)
    args.add("--output", output_file)
    args.add("--quiet")

    ctx.actions.run(
        mnemonic = "MeshCompiler",
        executable = ctx.executable._compiler,
        arguments = [args],
        progress_message = "{}: compiling {} to {}".format(ctx.label.name, input_file.path, output_file_name),
        inputs = [input_file],
        outputs = [output_file],
    )

    return [
        DefaultInfo(
            files = depset([output_file]),
            runfiles = ctx.runfiles(files = [output_file]),
        ),
        MeshInfo(source_file = input_file),
    ]

bengine_obj_mesh = rule(
    implementation = _bengine_obj_mesh_impl,
    attrs = {
        "src": attr.label(allow_single_file = [".obj"]),
        "output_prefix_path": attr.string(),
        "_compiler": attr.label(
            default = Label("//assets/compilers/model:model_compiler"),
            allow_single_file = True,
            executable = True,
            cfg = "exec",
        ),
    },
)
