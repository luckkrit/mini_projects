static int safe_filename(const char *s) {          /* letters, digits, . _ - / only */
    if (!*s) return 0;
    for (; *s; s++)
        if (!isalnum((unsigned char)*s) && !strchr("._-/", *s)) return 0;
    return 1;
}

/* returns number of players, or -1 on error */
static int load_players_json(const char *path, Player *out, int max) {
    if (!safe_filename(path)) return -1;

    char cmd[256];
    snprintf(cmd, sizeof cmd, "python3 bulk.py %s", path);
    FILE *r = popen(cmd, "r");
    if (!r) return -1;

    char line[512];
    int n = 0, ok = 1;
    while (fgets(line, sizeof line, r)) {
        if (n >= max || parse_player(line, &out[n]) < 0) { ok = 0; break; }
        n++;
    }

    int status = pclose(r);                        /* waits for Python to finish */
    if (!ok || status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return -1;
    return n;
}