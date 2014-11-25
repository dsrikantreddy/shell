// TODO fg, bg processes (&), jobs
// TODO strip '* ' from git prompt, put () in function, error case, unicode
// TODO command tab-completion
// TODO simple redirection
// TODO arithmetic?
// TODO feature toggle(runtime config?)
// TODO shells vars dict
// TODO control statements
#include "shell.h"

char cmd_error = CMD_OKAY;
int child_pid;
const char *home;
char input[INPUT_BUF_SIZE];
char *prompt;
char **opts;
char *tok;
int optCount;
char old_pwd[DIR_NAME_MAX_SIZE];

static void sighandler(int signo) {
    if (signo == CMD_ERROR_SIGNAL) {
        cmd_error = CMD_ERROR;
    }
    else if (signo == SIGINT) {
        if (child_pid) {
            kill(child_pid, SIGINT);
        }
        // If no child process or child process killed
        if (!child_pid || kill(child_pid, 0) < 0) { // Returns -1 on error
            // TODO make this async-safe
            // Re-display prompt
            char *prompt = (char *) malloc(PROMPT_MAX_SIZE * sizeof(char));
            get_prompt(prompt, PROMPT_MAX_SIZE);
            write(STDOUT_FILENO, "\n", 1);
            write(STDOUT_FILENO, prompt, strlen(prompt));
            free(prompt);
        }
    }
}

void print_error() {
    if (errno) {
        printf("[Error %d]: %s\n", errno, strerror(errno));
    }
}

void cd(const char *target) {
    // Duplicate the target; if the target points to old_pwd, we
    // don't want to overwrite the old_pwd
    char dup_target[DIR_NAME_MAX_SIZE];
    strncpy(dup_target, target, sizeof(dup_target));
    dup_target[sizeof(dup_target) - 1] = '\0';
    // Store the previous old_pwd in case chdir fails and we have to revert
    char prev_old_pwd[DIR_NAME_MAX_SIZE];
    strncpy(prev_old_pwd, old_pwd, sizeof(prev_old_pwd));
    prev_old_pwd[sizeof(prev_old_pwd) - 1] = '\0';
    getcwd(old_pwd, sizeof(old_pwd)); // Set the current dir as the new old_pwd
    if (chdir(dup_target) < 0) { // Returns -1 if error
        print_error();
        cmd_error = CMD_ERROR;
        strncpy(old_pwd, prev_old_pwd, sizeof(old_pwd)); // Restore previous old_pwd
        old_pwd[sizeof(old_pwd) - 1] = '\0';
    }
}

void cd_back() {
    cd(old_pwd);
}

char *get_user() {
    uid_t uid = geteuid();
    struct passwd *passwd = getpwuid(uid);
    if (passwd) {
        return passwd->pw_name;
    }
    return "Anon";
}

char *get_uid_symbol() {
    char *uid_symbol;
    const char non_root = '$';
    const char root = '#';
    if (getuid() != 0) {
        if (cmd_error == CMD_ERROR) {
            int required_size = sizeof(char) * (strlen(bold_prefix) + strlen(fg_red_160) + 1 + strlen(reset) + 1);
            uid_symbol = (char *) malloc(required_size);
            sprintf(uid_symbol, "%s%s%c%s", bold_prefix, fg_red_160, non_root, reset);
        }
        else {
            int required_size = sizeof(char) * (strlen(bold_prefix) + strlen(fg_white) + 1 + strlen(reset) + 1);
            uid_symbol = (char *) malloc(required_size);
            sprintf(uid_symbol, "%s%s%c%s", bold_prefix, fg_white, non_root, reset);
        }
        return uid_symbol;
    }
    else {
        int required_size = sizeof(char) * (strlen(bold_prefix) + strlen(fg_red_196) + 1 + strlen(reset) + 1);
        uid_symbol = (char *) malloc(required_size);
        sprintf(uid_symbol, "%s%s%c%s", bold_prefix, fg_red_196, root, reset);
        return uid_symbol;
    }
}

char *get_time_str(char *time_str_container) {
    time_t rawtime;
    time(&rawtime);
    struct tm *time = localtime(&rawtime);
    // TODO size check
    // Pad integer with zeroes to a length of 2
    if (sprintf(time_str_container, "%'02d:%'02d:%'02d", time->tm_hour, time->tm_min, time->tm_sec) < 0) {
        printf("[Error]: Error formatting time string.");
    }
    return time_str_container;
}

void abbreviate_home(char *full_path, size_t full_path_length) {
    // Replace $HOME with ~ in full_path
    char *match = strstr(full_path, home);
    if (match != NULL) {
        int path_size = (strlen(match) - strlen(home) + 2);
        char *trunc_path = (char *) malloc(path_size * sizeof(char *));
        trunc_path[0] = '~';
        trunc_path[1] = '\0';
        trunc_path = strncat(trunc_path, (char *) &match[strlen(home)], path_size - 2);
        trunc_path[path_size - 1] = '\0';
        strncpy(full_path, trunc_path, full_path_length);
        full_path[full_path_length - 1] = '\0';
        free(trunc_path);
    }
}

void execute() {
    if (optCount <= 0) {
        return;
    }

    // Debug info
    printf("cmd: %s\n", opts[0]);
    int u = 0;
    while (u <= optCount) {
        printf("opts[%d]: %s$\n", u, opts[u]);
        ++u;
    }
    printf("<~~~~~~~~ Output ~~~~~~~~>\n");

    // Handle built-in commands
    if (strcmp(opts[0], cmd_exit) == 0) {
        printf("Exiting...\n");
        free_all();
        exit(0);
    }
    else if (strcmp(opts[0], cmd_cd) == 0){
        if (opts[1] == NULL) {
            // By default, cd to home if no directory specified
            cd(home);
        }
        else {
            cd(opts[1]);
        }
    }
    else if (strcmp(opts[0], cmd_back) == 0) {
        cd_back();
    }
    else {
        // Fork to execute command
        child_pid = fork();
        if (!child_pid) {
            if (execvp(opts[0], opts) < 0) { // Returns -1 if error
                print_error();
                // Notify parent of error
                kill(getppid(), CMD_ERROR_SIGNAL);
            }
            // Note: child automatically exits after successful execvp
            exit(1);
        }
        else {
            int status;
            waitpid(child_pid, &status, 0);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status)) { // If exit status not 0
                    cmd_error = CMD_ERROR;
                }
            }
        }
    }
    printf("<~~~~ End of Output ~~~~~>\n");
}

void free_all() {
    // Free dynamically allocated memory
    free(prompt);
    if (optCount > 0) {
        for (;optCount >= 0;--optCount) {
            free(opts[optCount]);
        }
    }
    free(opts);
    free(tok);
}

void get_prompt(char *prompt, int prompt_max_size) {
    char cwd[DIR_NAME_MAX_SIZE];
    cwd[DIR_NAME_MAX_SIZE - 1] = '\0';
    // Get cwd
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        print_error();
    }
    // Generate prompt
    abbreviate_home(cwd, sizeof(cwd));
    char *time_str = (char *) malloc (DATE_MAX_SIZE * sizeof(char));
    get_time_str(time_str);
    char *uid_symbol = get_uid_symbol();
    char hostname[256];
    hostname[255] = '\0';
    gethostname(hostname, sizeof(hostname));
    char *git_container = git();
    snprintf(prompt, prompt_max_size, "%s%s[%s]%s %s%s%s%s%s%s@%s%s%s%s:%s%s%s%s%s %s%s%s%s %s\n%s%s>>%s ", bold_prefix, fg_red_196, time_str, reset, bold_prefix, fg_bright_green, get_user(), reset, bold_prefix, fg_blue_24, hostname, reset, bold_prefix, fg_bright_green, reset, bold_prefix, fg_blue_39, cwd, reset, bold_prefix, fg_green, git_container, reset, uid_symbol, bold_prefix, fg_green, reset);
    free(git_container);
    free(uid_symbol);
    free(time_str);
}

void parse_input(char input[INPUT_BUF_SIZE]) {
    // Initializations
    opts = (char **) malloc(sizeof(char *));
    tok = (char *) malloc(sizeof(char));
    tok[0] = '\0';
    optCount = 0;
    int i = 0, tokIndex = 0;
    char state = STATE_NORMAL;
    // Iterate through each char of input
    while (input[i]) {
        if ((input[i] != '\n' && input[i] != ' ') || (state == STATE_IN_QUOTES)) { // Ignore whitespace
            // Handle escape characters
            if (input[i] == '\\') {
                // Add char that follows the escape char to token
                tok = (char *) realloc(tok, (tokIndex + 2) * sizeof(char));
                tok[tokIndex] = input[++i]; // Advance past next index in input
                tok[++tokIndex] = '\0';
            }
            // Handle semicolons (multiple commands separator)
            else if (input[i] == ';') {
                // Execute commands as we parse input
                // Add last opt to opts array
                if (tok[0] != '\0') { // Make sure an argument exists to add
                    opts = (char **) realloc(opts, (optCount + 1) * sizeof(char *));
                    opts[optCount] = (char *) malloc((strlen(tok) + 1) * sizeof(char));
                    // Copy token to opts and add null terminator
                    strncpy(opts[optCount], tok, strlen(tok));
                    opts[optCount][strlen(tok)] = '\0';
                    // Increment optCount counter
                    ++optCount;
                }

                // Add required NULL argument for exec
                opts = (char **) realloc(opts, (optCount + 1) * sizeof(char *));
                opts[optCount] = NULL;

                execute();
                // Reset tok
                tok[0] = '\0';
                // Reset tokIndex
                tokIndex = 0;
                // Reset opts
                if (optCount > 0) {
                    for (;optCount >= 0;--optCount) {
                        free(opts[optCount]);
                    }
                }
                free(opts);
                // Reinstantiate opts
                opts = (char **) malloc(sizeof(char *));
                // Reset optCount
                optCount = 0;
            }
            // Interpret words in quotes as a single token
            else if (input[i] == '\"') {
                if (state != STATE_IN_QUOTES) {
                    printf("Entering quotes\n");
                    state = STATE_IN_QUOTES;
                }
                else {
                    printf("Exiting quotes\n");
                    state = STATE_NORMAL;
                }
            }
            // Substitute ~ with $HOME, if applicable
            else if (input[i] == '~') {
                // Allocate memory for $HOME in tok
                tok = (char *) realloc(tok, (tokIndex + strlen(home) + 1) * sizeof(char));
                // Add $HOME to token
                tok = strcat(tok, home);
                // Add null terminator
                tok[tokIndex + strlen(home)] = '\0';
                // Update tokIndex
                tokIndex += strlen(home);
            }
            // Copy char to var tok
            else {
                tok = (char *) realloc(tok, (tokIndex + 2) * sizeof(char));
                tok[tokIndex] = input[i];
                tok[++tokIndex] = '\0';
            }
        }
        // Case when we've reached the end of a word
        // tokIndex != 0 ensures that there is non-whitespace preceding this space
        else if (input[i] == ' ' && tokIndex != 0) {
            opts = (char **) realloc(opts, (optCount + 1) * sizeof(char *));
            // Copy token to opts array
            opts[optCount] = (char *) malloc((strlen(tok) + 1) * sizeof(char));
            strncpy(opts[optCount], tok, strlen(tok));
            opts[optCount][strlen(tok)] = '\0';
            // Reset tok
            tok[0] = '\0';
            // Reset tokIndex to 0
            tokIndex = 0;
            // Increment optCount to keep track of num of opts
            ++optCount;
        }
        ++i;
    }
    cmd_error = CMD_OKAY; // Reset the error flag
    // If a command was supplied, then try to execute it
    // !(optCount == 0 && tokIndex == 0) ensures that there was
    // at least one non-whitespace character in the input
    if (i >= 1 && !(optCount == 0 && tokIndex == 0)) {
        // Add last opt to opts array
        if (tok[0] != '\0') { // Make sure an argument exists to add
            opts = (char **) realloc(opts, (optCount + 1) * sizeof(char *));
            opts[optCount] = (char *) malloc((strlen(tok) + 1) * sizeof(char));
            // Copy token to opts and add null terminator
            strncpy(opts[optCount], tok, strlen(tok));
            opts[optCount][strlen(tok)] = '\0';
            // Increment optCount counter
            ++optCount;
        }

        // Add required NULL argument for exec
        opts = (char **) realloc(opts, (optCount + 1) * sizeof(char *));
        opts[optCount] = NULL;

        execute();
    }
    else {
        cmd_error = CMD_BLANK;
    }
}

void git_branch(char *container, size_t container_size) {
    char *l_opts[3] = {"git", "branch", NULL};
    int pipes[2];
    if (pipe(pipes) < 0) { // Returns -1 if error
        print_error();
        return;
    }
    // Fork to execute command
    child_pid = fork();
    if (!child_pid) {
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[0]);
        freopen("/dev/null", "w", stderr); // Redirect stderr to /dev/null
        if (execvp(l_opts[0], l_opts) < 0) { // Returns -1 if error
            container[0] = '\0';
        }
        // Note: child automatically exits after successful execvp
        exit(1);
    }
    else {
        close(pipes[1]);
        char output[1024];
        int bytes = read(pipes[0], output, sizeof(output));
        if (bytes == 0) { // Git error (e.g. not in git repo)
            container[0] = '\0';
            return;
        }
        output[bytes] = '\0';
        int i = 0;
        int container_index = 0;
        for (;output[i];++i) {
            if (output[i] == '*' && output[i + 1] == ' ') {
                i += 2; // Advance past * and space
                while (output[i] && output[i] != '\n' && (container_index < container_size - 2)) {
                    container[container_index++] = output[i];
                    ++i;
                }
                break;
            }
        }
        container[container_index] = '\0';
        wait(NULL);
    }
}

void git_status(char *container, size_t container_size) {
    if (container_size < GIT_STATUS_MAX_SIZE) {
        fprintf(stderr, "[Error]: Incorrect size for git status container.");
        return;
    }
    char *l_opts[3] = {"git", "status", NULL};
    int pipes[2];
    if (pipe(pipes) < 0) { // Returns -1 if error
        print_error();
        return;
    }
    // Fork to execute command
    child_pid = fork();
    if (!child_pid) {
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[0]);
        freopen("/dev/null", "w", stderr); // Redirect stderr to /dev/null
        if (execvp(l_opts[0], l_opts) < 0) { // Returns -1 if error
            container[0] = '\0';
        }
        // Note: child automatically exits after successful execvp
        exit(0);
    }
    else {
        close(pipes[1]);
        char output[4096];
        int bytes = read(pipes[0], output, sizeof(output));
        if (bytes == 0) { // Git error (e.g. not in git repo)
            container[0] = '\0';
            return;
        }
        output[bytes] = '\0';
        int container_index = 0;
        if (strstr(output, "Changes to be committed") != NULL) {
            sprintf(container, " %s", cross);
            container_index += strlen(cross) + 1;
        }
        else {
            sprintf(container, " %s", check);
            container_index += strlen(check) + 1;
        }
        if (strstr(output, "Changes not staged for commit") != NULL) {
            sprintf(container, "%s %s", container, delta);
            container_index += strlen(delta) + 1;
        }
        container[container_index] = '\0';
        wait(NULL);
    }
}

char *git() {
    char *git_branch_container = (char *) malloc(GIT_BRANCH_MAX_SIZE * sizeof(char));
    int container_size = GIT_BRANCH_MAX_SIZE + GIT_STATUS_MAX_SIZE;
    char *git_container = (char *) malloc(container_size * sizeof(char));
    git_container[0] = '\0';
    git_branch(git_branch_container, GIT_BRANCH_MAX_SIZE);
    if (strlen(git_branch_container) != 0) { // If in valid git repo
        char *git_status_container = (char *) malloc(GIT_STATUS_MAX_SIZE * sizeof(char));
        git_status(git_status_container, GIT_STATUS_MAX_SIZE);
        snprintf(git_container, container_size, "(%s%s)", git_branch_container, git_status_container);
        git_container[container_size - 1] = '\0';
        free(git_status_container);
    }
    free(git_branch_container);
    return git_container;
}

int main() {
    signal(CMD_ERROR_SIGNAL, sighandler);
    signal(SIGINT, sighandler);
    // TODO allow for possible changing home dir
    home = getenv("HOME");
    getcwd(old_pwd, sizeof(old_pwd));
    while (!feof(stdin)) {
        // Initializations
        prompt = (char *) malloc(PROMPT_MAX_SIZE * sizeof(char));

        get_prompt(prompt, PROMPT_MAX_SIZE);
        char *line = readline(prompt);
        if (line == NULL) {
            printf("\n[Reached EOF]\n");
            free(line);
            free(prompt);
            exit(0);
        }
        strncpy(input, line, INPUT_BUF_SIZE);
        input[INPUT_BUF_SIZE - 1] = '\0';
        printf("input: %s\n", input);

        parse_input(input);
        if (cmd_error == CMD_OKAY) {
            add_history(input);
        }
        free(line);
        free_all();
    }
    return 0;
}

