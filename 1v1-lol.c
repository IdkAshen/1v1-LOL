#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAP_SIZE 20
#define TOTAL_PLAYERS 100
#define LOOT_POOL_SIZE 4

typedef struct {
    char name[30];
    int damage;
} Weapon;

typedef struct {
    char name[30];
    int hp;
    int shield;
    int x;
    int y;
    int wood;
    Weapon weapon;
    int is_bot;
    int alive;
} Player;

void delay_ms(int ms) {
    usleep(ms * 1000);
}

int get_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

void take_damage(Player *p, int amount) {
    if (p->shield >= amount) {
        p->shield -= amount;
    } else {
        amount -= p->shield;
        p->shield = 0;
        p->hp -= amount;
    }
    if (p->hp <= 0) {
        p->hp = 0;
        p->alive = 0;
    }
}

int main() {
    srand(time(NULL));

    // Weapons Pool
    Weapon loot_pool[LOOT_POOL_SIZE] = {
        {"Common Assault Rifle", 20},
        {"Rare SMG", 15},
        {"Epic Pump Shotgun", 45},
        {"Legendary SCAR", 35}
    };

    // Instantiate Human Player
    Player human;
    strcpy(human.name, "You (ProCoder)");
    human.hp = 100;
    human.shield = 0;
    human.wood = 0;
    human.weapon = loot_pool[0]; // Start with basic
    human.is_bot = 0;
    human.alive = 1;

    // Instantiate 99 Bots Array
    Player lobby[TOTAL_PLAYERS - 1];
    for (int i = 0; i < TOTAL_PLAYERS - 1; ++i) {
        sprintf(lobby[i].name, "Bot_%d", i + 1);
        lobby[i].hp = 100;
        lobby[i].shield = (rand() % 2) * 50;
        lobby[i].x = rand() % MAP_SIZE;
        lobby[i].y = rand() % MAP_SIZE;
        lobby[i].wood = rand() % 20;
        lobby[i].weapon = loot_pool[rand() % LOOT_POOL_SIZE];
        lobby[i].is_bot = 1;
        lobby[i].alive = 1;
    }

    int storm_radius = MAP_SIZE;
    int center_x = MAP_SIZE / 2;
    int center_y = MAP_SIZE / 2;

    printf("=== WELCOME TO FORTNITE (C ADVANCED EDITION) ===\n");
    printf("🚎 The Battle Bus is flying over a %dx%d grid island...\n\n", MAP_SIZE, MAP_SIZE);
    delay_ms(1000);

    // Drop Phase
    printf("Enter your landing coordinates (X Y) from 0 to %d: ", MAP_SIZE - 1);
    if (scanf("%d %d", &human.x, &human.y) != 2) {
        human.x = 0; human.y = 0;
    }
    if (human.x < 0 || human.x >= MAP_SIZE) human.x = 0;
    if (human.y < 0 || human.y >= MAP_SIZE) human.y = 0;

    // Loot Phase
    printf("\n📦 Landing... Opening a chest!\n");
    delay_ms(1500);
    human.weapon = loot_pool[rand() % LOOT_POOL_SIZE];
    human.shield = 100;
    human.wood = 30;
    printf("Found: %s (%d Dmg)!\n", human.weapon.name, human.weapon.damage);
    printf("Found: 2x Shield Pots (+100 Shield)!\n");
    printf("Found: Materials (+30 Wood)!\n");
    delay_ms(1500);

    // Main Game Loop
    while (human.alive) {
        // Count active survivors
        int alive_count = 1; // Count human
        for (int i = 0; i < TOTAL_PLAYERS - 1; ++i) {
            if (lobby[i].alive) alive_count++;
        }

        if (alive_count == 1) break; // Victory Royale condition

        printf("\n================================================\n");
        printf("👑 SURVIVORS: %d / %d\n", alive_count, TOTAL_PLAYERS);
        printf("💖 HP: %d | 🛡️ SHIELD: %d | 🪵 WOOD: %d\n", human.hp, human.shield, human.wood);
        printf("📍 LOCATION: (%d, %d) | 🔫 WEAPON: %s\n", human.x, human.y, human.weapon.name);
        printf("⛈️ STORM EYE: Radius %d around (%d, %d)\n", storm_radius, center_x, center_y);
        printf("================================================\n");
        delay_ms(1000);

        // 1. Storm Logic
        if (get_distance(human.x, human.y, center_x, center_y) > storm_radius) {
            printf("⛈️ You are in the STORM! Taking 20 damage!\n");
            take_damage(&human, 20);
            if (!human.alive) break;
        }

        // Simulate off-screen bot matches to thin the lobby size quickly
        int passive_eliminations = rand() % 15 + 5;
        for (int i = 0; i < TOTAL_PLAYERS - 1; ++i) {
            if (lobby[i].alive && passive_eliminations > 0 && (rand() % 3 == 0)) {
                lobby[i].alive = 0;
                passive_eliminations--;
            }
        }

        // 2. Action Input
        printf("\nWhat is your next play?\n");
        printf("1. Move toward the Safe Zone\n");
        printf("2. Farm Wood (+20 Wood)\n");
        printf("3. Hunt for nearby players\n");
        printf("Choice: ");
        int choice;
        if (scanf("%d", &choice) != 1) choice = 1;

        if (choice == 1) {
            if (human.x < center_x) human.x++; else if (human.x > center_x) human.x--;
            if (human.y < center_y) human.y++; else if (human.y > center_y) human.y--;
            printf("🏃 You rotated toward the safe zone center. Now at (%d, %d)\n", human.x, human.y);
        } else if (choice == 2) {
            human.wood += 20;
            printf("🪓 You hit some trees. Total Wood: %d\n", human.wood);
        } else {
            printf("👀 Scanning the horizon for movement...\n");
        }
        delay_ms(1000);

        // 3. Matchmaker Scanner (Find nearest bot)
        Player *enemy = NULL;
        for (int i = 0; i < TOTAL_PLAYERS - 1; ++i) {
            if (lobby[i].alive && get_distance(human.x, human.y, lobby[i].x, lobby[i].y) <= 3) {
                enemy = &lobby[i];
                break;
            }
        }

        // 4. Combat Calculations
        if (enemy != NULL) {
            printf("\n❗ SPOTTED: %s is pushing your position!\n", enemy->name);
            delay_ms(1000);

            // Building Defense Engine
            if (human.wood >= 10) {
                human.wood -= 10;
                printf("🪵 You quickly built a wooden wall! (Absorbs enemy's first strike)\n");
                printf("🔫 You counter-attack with your %s!\n", human.weapon.name);
                take_damage(enemy, human.weapon.damage);
                printf("💥 Hit! %s HP is down to %d\n", enemy->name, enemy->hp);
            } else {
                printf("❌ Out of materials! Direct shootout!\n");
                printf("🔫 You shoot your %s!\n", human.weapon.name);
                take_damage(enemy, human.weapon.damage);
                
                printf("💥 %s lasers you back with %s!\n", enemy->name, enemy->weapon.name);
                take_damage(&human, enemy->weapon.damage);
            }

            if (!enemy->alive) {
                printf("💀 ELIMINATED! You knocked out %s and took their shield cells!\n", enemy->name);
                human.shield += 50;
                if (human.shield > 100) human.shield = 100;
            }
            delay_ms(1500);
        } else {
            printf("💨 Quiet turn. No nearby firefights found.\n");
            // Bots migrate to safe zone center
            for (int i = 0; i < TOTAL_PLAYERS - 1; ++i) {
                if (lobby[i].alive) {
                    if (lobby[i].x < center_x) lobby[i].x++; else if (lobby[i].x > center_x) lobby[i].x--;
                    if (lobby[i].y < center_y) lobby[i].y++; else if (lobby[i].y > center_y) lobby[i].y--;
                }
            }
        }

        // 5. Shrink Circle
        if (storm_radius > 2) {
            storm_radius -= 2;
            printf("\n⚡ The Storm is shrinking! Safe zone is getting tighter! ⚡\n");
        }
        delay_ms(1000);
    }

    // Results Processing
    printf("\n================================================\n");
    if (human.alive) {
        printf("🎉 🎉 🎉 #1 VICTORY ROYALE! 🎉 🎉 🎉\n");
        printf("You conquered the 100-player lobby in pure C!\n");
    } else {
        printf("☠️ You were eliminated. Better luck next match! ☠️\n");
    }
    printf("================================================\n");

    return 0;
}
