export def "cmake-update" [] {
    rg -l --glob='CMakeLists.txt' -Fe '[[[cog'
        | lines
        | par-each { |p| cog -I . -r $p }
        | parse --regex '^Cogging (?<path>.*?)(?:\s+\((?<status>changed)\))?$'
}

# Copies all CMake files and cmake-related resources from the current project
# to the specified Git worktree.
export def "cmake-copy" [
    target_dir: path  # Target Git worktree path
    --dry-run         # Print what would be copied without making changes
] {
    # Resolve target to an absolute path before changing directory
    let target_dir = $target_dir | path expand
    let source_dir = git rev-parse --show-toplevel | str trim
    cd $source_dir
    print $"Source directory: ($source_dir)"
    print $"Target directory: ($target_dir)"

    if not ($target_dir | path join ".git" | path exists) {
        error make { msg: "Target directory doesn't appear to be a Git repository or worktree." }
    }

    if $dry_run {
        print "DRY RUN: No files will be copied"
    }

    let extra_files = [
        cmake_builders.py
        CMakePresets.json
        misc/scripts/godotdev.nu
        core/profiling/profiling.gen.h.in
        .lldbinit
        dev.nu
    ]

    # Ignore patterns from .cmake-copy-ignore (git pathspec syntax, # comments allowed)
    let excludes = if (".cmake-copy-ignore" | path exists) {
        open --raw .cmake-copy-ignore
        | lines
        | str trim
        | where {|line| $line != "" and not ($line | str starts-with "#") }
        | each {|line| ":(exclude)" + $line }
    } else {
        []
    }

    # git ls-files handles gitignore for us: tracked + untracked-but-not-ignored
    let cmake_files = (
        git ls-files --cached --others --exclude-standard --
            'CMakeLists.txt' '*/CMakeLists.txt' '*.cmake' 'cmake/' ...$excludes
        | lines
    )

    let extras = $extra_files | where {|file|
        if ($file | path exists) {
            true
        } else {
            print -e $"Warning: ($file) not found in source directory"
            false
        }
    }

    $cmake_files ++ $extras
    | uniq
    | sort
    | to text
    | rsync --checksum --itemize-changes ...(if $dry_run { [--dry-run] } else { [] }) --files-from=- $"($source_dir)/" $"($target_dir)/"

    if $dry_run {
        print "Dry run complete. No files were copied."
    } else {
        print "Operation complete."
    }
}
