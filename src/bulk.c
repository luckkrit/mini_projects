#include "models.h"

static int safe_filename(const char *s) {          /* letters, digits, . _ - / only */
    if (!*s) return 0;
    for (; *s; s++)
        if (!isalnum((unsigned char)*s) && !strchr("._-/", *s)) return 0;
    return 1;
}
static int parse_player(char *line, Player *p) {
    line[strcspn(line, "\r\n")] = '\0';

    char *f[4], *cur = line;
    for (int i = 0; i < 3; i++) {            /* split on the first 3 tabs */
        f[i] = cur;
        char *t = strchr(cur, '\t');
        if (!t) return -1;
        *t = '\0';
        cur = t + 1;
    }
    f[3] = cur;
    if (strchr(f[3], '\t')) return -1;       /* too many fields */

    if (strlen(f[0]) >= sizeof p->playerId || strlen(f[1]) >= sizeof p->playerName)
        return -1;

    char *end;
    long num = strtol(f[2], &end, 10);
    if (*end != '\0' || num < 1 || num > 99) return -1;
    double price = strtod(f[3], &end);
    if (*end != '\0' || price < 0) return -1;

    snprintf(p->playerId, sizeof p->playerId, "%s", f[0]);
    snprintf(p->playerName, sizeof p->playerName, "%s", f[1]);
    p->squadNumber = (int)num;
    p->price = price;
    return 0;
}
/* returns number of players, or -1 on error */
static int load_players_json(const char *path, Player *out, int max) {
    if (!safe_filename(path)) return -1;

    char cmd[256];
    snprintf(cmd, sizeof cmd, "python3 src/bulk.py %s", path);
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

int main(){
    Player out[MAX_PLAYER] = {0};
    load_players_json("src/data.json", out, MAX_PLAYER);
    for(int i=0;i<MAX_PLAYER;i++){
        if(out[i].playerId[0] == '\0') continue;
        printf("%s %s %d %f\n", out[i].playerId, out[i].playerName, out[i].squadNumber, out[i].price);
    }
    return 0;
}