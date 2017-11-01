# Contributing Guidelines

Some basic conventions for contributing to this project.

### General

Please make sure there are no existing pull requests to address the mentioned new features or issues. And, please check for issues related to the update, as someone else may be working on the issue.

* Please open a discussion in a new issue / existing issue to talk about the changes you'd like to bring
* Develop in a topic branch, not master/develop

When creating a new branch, prefix it with the *type* of the change (see section **Commit Message Format** below), the associated opened issue number, a dash and some text describing the issue (using dash as a separator).

For example, if you work on a bugfix for the issue #111, you could name the branch `fix111-scan-function`.

### Commit Message Format

Each commit message should include a **type**, a **scope** and a **subject**:

```
 <type>(<scope>): <subject>
```

Lines should not exceed 100 characters. This allows the message to be easier to read on github as well as in various git tools and produces a nice, neat commit log ie:

```
 #112 feat(network-scan): add network vendor informations
 #113 fix(network-scan): fix a display issue in ui view
```

#### Type

Must be one of the following:

* **feat**: A new feature
* **fix**: A bug fix
* **docs**: Documentation only changes
* **style**: Changes that do not affect the meaning of the code "CatchME UI" (white-space, formatting, missing
  semi-colons, etc)
* **refactor**: A code change that neither fixes a bug or adds a feature

#### Scope

The scope could be anything specifying place of the commit change. For example `networks`,
`containers`, `images` etc.
You can use the **area** label tag associated on the issue here (for `ui/network-scan` use `network-scan` as a scope)

#### Subject

The subject contains succinct description of the change:

* use the imperative, present tense: "change" not "changed" nor "changes"
* don't capitalize first letter
* no dot (.) at the end