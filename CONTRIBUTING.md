# Contributing to ArgusCore

So, you want to contribute? Great!
Contributing is not only about creating fixes, but also reporting bugs. Before reporting a bug, please make sure you are using the latest core and database revision.


## Reporting Issues

Use the issue templates — they will guide you through the required information:

- **Bug Report** — crashes, broken mechanics, logic errors
- **Database Bug Report** — wrong NPC/quest/item/spell/GO data
- **Feature Request** — new features or improvements

Before creating a ticket, check whether a report for the same bug already exists to avoid duplicates.


## Creating Pull Requests

1. Fork the repository
2. Create a branch: `git checkout -b my-fix`
3. Make your change — keep it small and focused, one fix per PR
4. Commit following the convention: `fix(DB/World): describe what changed and why`
5. Push: `git push origin my-fix`
6. Open a Pull Request against the main branch — the PR template will guide you

**Guidelines:**
- Follow the existing code style of the file you are editing
- For C++ fixes: one branch per fix so you can submit independent PRs without blocking each other
- For SQL files: name them `YYYY_MM_DD_NN_database.sql` and place them under `sql/updates/<database>/master/`
- When changing `auth` or `characters` schemas, update the base files under `sql/base/` as well
- SQL-only fixes can be submitted as standalone PRs

See [ROADMAP.md](ROADMAP.md) for current development priorities and what kinds of changes are in scope for each phase.


## Requirements

Software requirements are listed in [README.md](README.md).
