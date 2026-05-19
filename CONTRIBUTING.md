# Contributing to ArgusCore

So, you want to contribute? Great!
Contributing is not only about creating fixes, but also reporting bugs. Before reporting a bug, please make sure you are using the latest core and database revision.


## Reporting Issues

Before creating a ticket, check whether a report for the same bug already exists to avoid duplicates.

Mandatory information when creating a ticket:

- Branch name
- Commit hash (run `git log -1 --format="%H %ai"` in the repo root)
- Affected creature / item / quest ID and a link to the relevant wowhead page
- Clear title and description of the bug
- If reporting a crash: compile in Debug mode — Release builds do not include enough symbol information to be useful


## Creating Pull Requests

1. Fork the repository
2. Create a branch: `git checkout -b my-fix`
3. Make your change — keep it small and focused, one fix per PR
4. Commit: `git commit -am "Fix: describe what the change does and why"`
5. Push: `git push origin my-fix`
6. Open a Pull Request against the main branch

**Guidelines:**
- Follow the existing code style of the file you are editing
- For C++ fixes: one branch per fix so you can submit independent PRs without blocking each other
- For SQL files accompanying a C++ fix: name them `YYYY_MM_DD_i_database.sql`
- When changing `auth` or `characters` schemas, update the base files under `sql/base/` as well
- SQL-only fixes can be submitted as standalone PRs

See [ROADMAP.md](ROADMAP.md) for the current development priorities and what kinds of changes are in scope for each phase.


## Requirements

Software requirements are listed in [README.md](README.md).
