def _combine_natviz_files_impl(ctx):
    input_files = ctx.files.srcs
    output_file = ctx.actions.declare_file("{}.natvis".format(ctx.label.name))

    args = ctx.actions.args()
    for file in input_files:
        args.add("--input", file)

    args.add("--output", output_file)

    ctx.actions.run(
        mnemonic = "NatvizCombiner",
        executable = ctx.executable._compiler,
        arguments = [args],
        progress_message = "Combining natvis files",
        inputs = input_files,
        outputs = [output_file],
    )

    return [DefaultInfo(files = depset([output_file]))]

combine_natviz_files = rule(
    implementation = _combine_natviz_files_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".natvis"]),
        "_compiler": attr.label(
            default = Label("//debug:natviz_combiner"),
            allow_single_file = True,
            executable = True,
            cfg = "exec",
        ),
    },
)
