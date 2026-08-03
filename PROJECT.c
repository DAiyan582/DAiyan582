#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_BALLS 300  // 50 overs * 6 balls

// Structures
struct player {
    char name[50];
    int runs_scored;
    int balls_faced;
    int fours;
    int sixes;
    float strike_rate;
    int out; // 0 = Not Out, 1 = Out
    char dismissal[30];
};

struct match {
    char team_name[50];
    int total_runs;
    int total_wickets;
    int total_balls;
    float overs_played;
    struct player batsmen[11];
    int current_batsman_index;
    int next_batsman_index;
};

// Function prototypes
void init_match(struct match *m);
void add_batsman(struct match *m, int index, char *name);
void print_scorecard(struct match m, int final);
void generate_ball_event(struct match *m, int *runs_scored, int *is_wicket, char *dismissal);
void print_live_score(struct match m);
void save_scorecard_to_file(struct match m, int final);

int main() {
    struct match m;
    init_match(&m);

    printf("============================================\n");
    printf("     CRICKET MATCH SIMULATOR (50 OVERS)    \n");
    printf("============================================\n\n");
    printf("Enter team name: ");
    scanf("%s", m.team_name);

    // Input players
    printf("\nEnter 11 players for %s:\n", m.team_name);
    for (int i = 0; i < 11; i++) {
        printf("Player %d: ", i + 1);
        char name[50];
        scanf("%s", name);
        add_batsman(&m, i, name);
    }

    printf("\n--- MATCH STARTED ---\n");
    m.current_batsman_index = 0; // Player 1 is batting
    m.next_batsman_index = 1;    // Player 2 is waiting

    // Main Game Loop
    while (m.total_wickets < 10 && m.total_balls < MAX_BALLS) {
        int runs = 0;
        int is_wicket = 0;
        char dismissal[30] = "";

        // Simulate a ball
        generate_ball_event(&m, &runs, &is_wicket, dismissal);

        // Update current batsman stats
        struct player *current = &m.batsmen[m.current_batsman_index];
        current->balls_faced++;
        
        if (runs == 4) current->fours++;
        if (runs == 6) current->sixes++;
        
        if (is_wicket) {
            current->out = 1;
            strcpy(current->dismissal, dismissal);
            m.total_wickets++;
            
            // Move to next batsman if available
            if (m.next_batsman_index < 11) {
                m.current_batsman_index = m.next_batsman_index;
                m.next_batsman_index++;
            } else {
                // All out
                break;
            }
        } else {
            current->runs_scored += runs;
            m.total_runs += runs;
            // Strike rate update
            current->strike_rate = (current->balls_faced > 0) ? 
                (float)current->runs_scored / current->balls_faced * 100.0 : 0.0;
        }

        m.total_balls++;
        m.overs_played = m.total_balls / 6.0;

        // Show live score after every over (6 balls) or wicket
        if (m.total_balls % 6 == 0 || is_wicket) {
            print_live_score(m);
        }
    }

    printf("\n============================================\n");
    printf("   INNINGS OVER! FINAL SCORECARD\n");
    printf("============================================\n");
    print_scorecard(m, 1);
    save_scorecard_to_file(m, 1);

    return 0;
}

// Initialize match
void init_match(struct match *m) {
    m->total_runs = 0;
    m->total_wickets = 0;
    m->total_balls = 0;
    m->overs_played = 0.0;
    m->current_batsman_index = 0;
    m->next_batsman_index = 1;
    srand(time(0)); // Random seed
}

// Add batsman
void add_batsman(struct match *m, int index, char *name) {
    strcpy(m->batsmen[index].name, name);
    m->batsmen[index].runs_scored = 0;
    m->batsmen[index].balls_faced = 0;
    m->batsmen[index].fours = 0;
    m->batsmen[index].sixes = 0;
    m->batsmen[index].strike_rate = 0.0;
    m->batsmen[index].out = 0;
    strcpy(m->batsmen[index].dismissal, "Not Out");
}

// Generate a ball event (Advanced logic)
void generate_ball_event(struct match *m, int *runs_scored, int *is_wicket, char *dismissal) {
    int event = rand() % 100;

    // Extras (No ball, Wide) - 10% chance
    if (event < 5) { // Wide
        *runs_scored = 1;
        *is_wicket = 0;
        m->total_runs += 1; // Extra added to team total
        m->total_balls--;   // Doesn't count as a legal ball
        return;
    } else if (event >= 5 && event < 10) { // No-ball + usually runs
        *runs_scored = 1 + (rand() % 4); // 1 to 4 runs
        *is_wicket = 0;
        m->total_runs += 1 + (rand() % 4);
        m->total_balls--;
        return;
    }

    // Legal Delivery
    if (event >= 10 && event < 30) { // 0 Runs (Dot Ball) - 20%
        *runs_scored = 0;
        *is_wicket = 0;
    } else if (event >= 30 && event < 50) { // 1 Run - 20%
        *runs_scored = 1;
        *is_wicket = 0;
    } else if (event >= 50 && event < 65) { // 2 Runs - 15%
        *runs_scored = 2;
        *is_wicket = 0;
    } else if (event >= 65 && event < 75) { // 3 Runs - 10%
        *runs_scored = 3;
        *is_wicket = 0;
    } else if (event >= 75 && event < 88) { // 4 Runs (Boundary) - 13%
        *runs_scored = 4;
        *is_wicket = 0;
    } else if (event >= 88 && event < 95) { // 6 Runs (Six) - 7%
        *runs_scored = 6;
        *is_wicket = 0;
    } else { // WICKET - 5% chance
        *runs_scored = 0;
        *is_wicket = 1;
        int dismiss = rand() % 5;
        if (dismiss == 0) strcpy(dismissal, "Bowled");
        else if (dismiss == 1) strcpy(dismissal, "Caught");
        else if (dismiss == 2) strcpy(dismissal, "LBW");
        else if (dismiss == 3) strcpy(dismissal, "Stumped");
        else strcpy(dismissal, "Run Out");
    }
}

// Print Live Score (Over-by-Over)
void print_live_score(struct match m) {
    printf("\n[Live] %s: %d/%d | Overs: %.1f | Current Bat: %s (%d runs)\n", 
        m.team_name, m.total_runs, m.total_wickets, m.overs_played, 
        m.batsmen[m.current_batsman_index].name, 
        m.batsmen[m.current_batsman_index].runs_scored);
}

// Print Final Scorecard
void print_scorecard(struct match m, int final) {
    printf("\n============================================\n");
    printf("Team: %s\n", m.team_name);
    printf("Total: %d/%d in %.1f overs\n", m.total_runs, m.total_wickets, m.overs_played);
    printf("============================================\n");
    printf("%-15s %-6s %-6s %-4s %-4s %-10s\n", "Batsman", "Runs", "Balls", "4s", "6s", "SR");
    printf("--------------------------------------------\n");
    
    for (int i = 0; i < 11; i++) {
        struct player p = m.batsmen[i];
        char status[15];
        if (p.out) {
            sprintf(status, "(c %s)", p.dismissal); // c = caught, b = bowled
        } else {
            strcpy(status, "(Not Out)");
        }
        printf("%-15s %-6d %-6d %-4d %-4d %-10.2f %s\n", 
            p.name, p.runs_scored, p.balls_faced, p.fours, p.sixes, p.strike_rate, status);
    }
    printf("============================================\n");
}

// Save to File
void save_scorecard_to_file(struct match m, int final) {
    FILE *fp = fopen("scorecard.txt", "w");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(fp, "============================================\n");
    fprintf(fp, "Team: %s\n", m.team_name);
    fprintf(fp, "Total: %d/%d in %.1f overs\n", m.total_runs, m.total_wickets, m.overs_played);
    fprintf(fp, "============================================\n");
    fprintf(fp, "Batsman\t\tRuns\tBalls\t4s\t6s\tSR\n");
    fprintf(fp, "--------------------------------------------\n");
    
    for (int i = 0; i < 11; i++) {
        struct player p = m.batsmen[i];
        char status[15];
        if (p.out) sprintf(status, "(c %s)", p.dismissal);
        else strcpy(status, "(Not Out)");
        
        fprintf(fp, "%s\t\t%d\t%d\t%d\t%d\t%.2f\t%s\n", 
            p.name, p.runs_scored, p.balls_faced, p.fours, p.sixes, p.strike_rate, status);
    }
    fprintf(fp, "============================================\n");
    fclose(fp);
    printf("\n[✓] Scorecard saved to 'scorecard.txt'.\n");
}