---
trigger: always_on
---
# LUNAR --FX Development Rules

## Rules

1. **GitHub repo and release always up to date**
   - Keep the GitHub repository synchronized with local changes
   - Ensure releases are created and updated regularly

2. **README always up to date**
   - Keep the README.md current with the latest information
   - Update links, descriptions, and installation instructions

3. **Never comments in code**
   - Keep code clean without comments
   - Only keep the main header/description at the top of files

4. **Always ask user for important decisions**
   - Consult the user before making significant changes
   - Get approval for major architectural decisions

5. **Always run commands automatically**
   - Execute build, test, and deployment commands automatically
   - Don't ask for permission to run safe commands
   - Only pause for user approval on potentially destructive operations

6. **Keep entire project consistent**
   - When changing names, update everywhere in the project
   - Maintain consistency across all files (headers, source, documentation, installer)
   - If a plugin name changes, update all references
