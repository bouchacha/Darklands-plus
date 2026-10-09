# Card scripts: a guide for modders

Darklands towns, castles, monasteries, mines and most quest encounters are **cards**: a
picture, a block of text and a menu of options. The card engine (overlay s043) draws a
card from `msgfiles\<card>.msg` and `pics\<pic>.pic`. A **card script** decides which
options are enabled, the chance of each option, what happens when one is chosen and which
card comes next. The card scripts are overlays s044 to s135, about 350 C files in
`src/c/s044 .. src/c/s135`. Each file compiles byte for byte to the original game code.

The C came from a decompiler, so it uses raw linker names (`D_e48a`,
`sub_s043_150b_03b0`). `include/darklands/card.h` gives about 280 of them readable
names. The header holds `#define` lines only, so a file written with the readable names
compiles to exactly the same object file.

    python3 -I tools/cardnames.py src/c/s070/s070_1838.c        # convert in place
    python3 -I tools/cardnames.py --check src/c/s0*/*.c src/c/s1*/*.c   # anything left to convert?
    python3 -I tools/cardnames.py --diff FILE.c                  # preview

`cardnames.py` adds `#include "darklands/card.h"` under the comment lines at the top
(`/* place: */ /* orig: */ /* flags: */`). It replaces the raw names in code and comments,
leaves string literals alone, and never moves a declaration, because MSC's code depends on
declaration order. Running it twice gives the same file. `tools/cport.py` finds the header
in `include/`, copies it into the DOS compile directory and adds its contents to the
compile-cache key.

## How a card script works

A card is one function that returns the next card number. The engine calls it, and the
script runs a loop until `next_card` changes:

1. `card_load(card_name, pic)` shows the picture and loads the card's text blocks.
2. Script start-up: set `next_card` to this card's own number and fill in the `opt_flag_k`
   values (1 = enabled, 2 = disabled). Set `card_items[]` / `card_saints[]` to the items
   and saints that may be used here, then call `items_usable()` / `saints_usable()`.
3. Loop: copy the flags into `opt_on[k]`, put the option handlers into `opt_fn[k]`,
   show the option list with `card_options(block)` and run `card_menu(card_window,
   card_window2)`. It returns the chosen option.
4. Call the chosen option's handler `opt_fn[k]()`. If the choice came from the item/saint
   pop-up box (`menu_popup`), the engine has set `menu_member` and `menu_slot` instead. A
   handler returns a `struct res { int a; int b; }`: `a` is the chance of success in
   percent and `b` is the member who acts (-2 = the whole party). The engine also calls
   handlers to show "The chance of success is N%" when the player holds Shift.
5. An action function for the option rolls (`rand_mod(100) <= chance`) and shows a result
   text block with `card_text(block, 0)`. It lets time pass (`advance_hours`), changes
   skills, money, items or events, and sets `next_card`.

Text blocks are numbered in order inside the `.msg` file. Text can contain `$variables`
such as `$LeaderName`, `$ChosenOneName`, `$Money1`, `$Number1`, `$Text1` and `$ItemName`.
The engine fills them from `var_*` (see card.h), so set `var_money1`, `var_chosen1` and the
others before you show the text.

Who is who: in every rules function, `who` is a party member (0..4), -1 is the leader
(`leader`) and -2 is every present member. Locations are indices into `locations`. Cities
are 0..0x5b, and the current one is `cur_loc` (with `city_rec` / `city_desc`).

## The readable API (summary; card.h has one comment per name)

| area | names |
|---|---|
| engine | `next_card`, `card_name`, `pic_buf`/`pic_buf2`, `pic_names`, `card_load`, `card_load_near`, `card_text`, `card_options`, `card_menu`, `card_window`, `options_block`, `msg_buf` |
| options | `opt_on[]`, `opt_fn[]`, `opt_flag_0..9`, `card_items[]`, `card_saints[]`, `items_usable`, `saints_usable`, `item_fn`, `saint_fn`, `menu_member`, `menu_slot`, `menu_popup` |
| text variables | `var_chosen1..5`, `var_money1..5`, `var_number1/2`, `var_text1..4`, `var_item`, `var_loc`, `var_loc2`, `var_support0..6`, `var_named` |
| shared strings | `str_success_assured`, `str_screwed_up`, `str_no_risk`, `str_enter_combat`, `quality_words`, ... |
| party / world | `party`, `party_size`, `leader`, `party_money`, `party_x/y`, `cur_loc`, `city_rec`, `locations`, `events`, `item_defs`, `cur_hour/day/month/year`, `best_member`, `best_value` |
| numbers, money, time | `rand_mod`, `clamp_range`, `clamp99`, `imax`, `imin`, `money_set`, `money_add`, `money_sub`, `money_cmp`, `money_copy`, `money_format`, `advance_hours`, `advance_days`, `wait_until_hour`, `hours_until`, `is_daylight`, `chance_text` |
| members | `member_skill`, `member_attr`, `member_attr_max`, `add_skill`, `add_attr`, `practice_skill`, `injure`, `party_max_skill`, `party_min_skill`, `party_avg_skill`, `party_max_attr`, ..., `count_items`, `has_item`, `add_item`, `take_item`, `lose_items`, `item_quality`, `invoke_saint`, `saint_chance_lo/hi` |
| locations | `loc_query`, `cur_loc_query`, `loc_set`, `loc_reputation`, `loc_reputation_adj`, `loc_hostile`, `loc_side_get/set`, `nearest_city`, `nearest_loc_type`, `give_reward` |
| timed events | `event_new`, `event_update`, `event_here`, `event_here_extend`, `event_at`, `event_type_at`, `event_value_at`, `event_find`, `event_remove`, `loc_fact`, `loc_fact_value`, ... |
| battle | `run_battle`, `battle_result`, `battle_mode`, `battle_enemy`, `battle_seed` |

A `?` in a card.h comment means the meaning is not fully certain. Names still in raw form
(for example `D_ee70`, `D_ee72`, `D_eb0a` and the script's own `sub_sNNN_xxxx_yyyy`
functions) are not understood yet. Timed events are records of `kind`, `type`, owner, location,
value and tag. They are how the game remembers that a city is closed, that you were thrown
out, or that a quest is running. Most cards only test them with `event_at(kind, cur_loc)` or
start them with `event_here(kind, hours)` (9999 hours = forever).

## Annotated example (s044 `$CityG01`, the city gate; shortened)

```c
#include "darklands/card.h"
...
int far sub_s044_19dd_0000(void)                /* the card */
{
    ...
    strcpy(card_name, "$CityG01");             /* msgfiles\$CityG01.msg */
    _fstrcpy((char far *)pic_buf, pic_names.f_0a57);
    card_load(card_name, (char far *)pic_buf);
    next_card = 2;                             /* our own number: loop while unchanged */
    opt_flag_0 = 1;  ...  opt_flag_6 = 1;      /* all options enabled */
    v_si = (loc_query(cur_loc, 2) / 3 + 1) * members;     /* toll: pfennigs per member */
    money_set(D_577c, 0, 0, v_si);
    money_copy(var_money1, D_577c);            /* "$Money1" in the text */
    if (money_cmp(party_money, D_577c) == -1)
        opt_flag_0 = 2;                        /* can't pay: "pay the toll" disabled */
    if (event_at(0xb, cur_loc))
        opt_flag_2 = 2;                        /* already tried to sneak in recently */
    card_items[0] = 0x5f;  card_saints[0] = 0x59;   /* items / saints usable here */
    opt_flag_3 = items_usable();
    opt_flag_4 = saints_usable();
    if (next_card == 2) {
        do {
            opt_on[0] = (char)opt_flag_0;  ...
            opt_fn[0] = sub_s044_19dd_0366;    /* chance handlers */
            opt_fn[1] = sub_s044_19dd_04b2;  ...
            card_options(0);                   /* text block 0 holds the option list */
            mouse_show();
            v_si = card_menu(card_window, card_window2);
            mouse_hide();
            l_a = !menu_popup ? (*opt_fn[v_si])() : (*D_eb0a[menu_slot])();
            switch (v_si) {
            case 1: sub_s044_19dd_03da(l_a.a); break;   /* l_a.a = chance in % */
            ...
            }
        } while (next_card == 2);
    }
    return next_card;
}

struct res far sub_s044_19dd_04b2(void)        /* chance of "talk your way in" */
{
    struct res r;
    ...
    v_si = imax(member_attr(leader, 5), member_skill(leader, 0xa));
    r.a = clamp_range(1, 0x63, cur_rep / 2 + v_si);  /* reputation of the city */
    r.b = leader;                              /* who acts (shown next to the chance) */
    return r;
}

int far sub_s044_19dd_03da(int p_6)            /* ...and what happens */
{
    if (rand_mod(0x64) <= p_6) {               /* success */
        card_text(2, 0);                       /* text block 2, wait for a key */
        practice_skill(leader, 0xa, 1, 0xa, 0);
        loc_reputation_adj(cur_loc, 1, 1);
        advance_hours(2);
        return next_card = is_daylight() ? 6 : 8;
    }
    event_here(0xa, 0xc);                      /* remember the failure for 12 hours */
    card_text(3, 0);
    opt_flag_1 = 2;                            /* option now disabled */
    advance_hours(1);
    next_card = is_daylight() ? 2 : 3;         /* stay at the gate */
}
```

(The real file writes `if/else` arms out in full and keeps the decompiler's local names.
Matching the original code needs that. Your mod does not.)

## Building a mod

1. Make a mod directory, for example `mymod/`, and copy the card file you want to change
   into it. Any directory layout works. Keep the `/* place: */`, `/* orig: */` and
   `/* flags: */` lines. They tell the build which original code the file replaces. Edit the
   copy and leave `src/c` alone.
2. Build:

       python3 -I tools/build.py <DARKLAND.EXE> --skip-analysis --mod --overlay mymod --out mygame.exe

   A file in `mymod` replaces the `src/c` file that has the same `place:` address. The
   `#include "darklands/card.h"` still resolves, because the compiler looks next to the
   file first and then in `include/`.
3. Rules for the edited code:
   - **Code may grow.** The overlays are re-laid out around the larger function.
   - **DGROUP is full.** The module's string literals (`_DATA`) may shrink but must not
     grow. Put new text in far data: `static char far s_mine[] = "...";`. Copy it with
     `_fstrcpy((char far *)msg_buf, s_mine);` and use `strcpy` only for the existing
     literals. A module that had no `_DATA` at all cannot get any. (In this tree,
     `tools/cport.py` still rejects the far segment that such a static creates (`X5_DATA`),
     so check that the build accepts it.)
   - Call only functions and use only variables that already exist: anything in card.h,
     `sub_...` / `vec_...` labels, and `D_xxxx` globals. Declare each one at the top of
     the file, e.g. `int far practice_skill();`.
   - Card numbers (`next_card = 0x62`) are the game's own. Look at existing scripts to find
     the card you want to jump to.
4. A mod build writes `build/symbols.json` from the re-laid-out game, so `tools/fmatch.py`
   then reports nonsense for the overlays (every function `DIFF@0`). Run a normal build
   afterwards, or give the mod build its own `--work` directory.

## Keeping files converted

New card files keep arriving from `tools/cardgen.py --promote`. Run `tools/cardnames.py`
on them, then run `python3 -I tools/fmatch.py FILE`, which must still say `FILE MATCH`. If
a file already uses one of the readable names for something else (a local variable or a
struct member), the tool refuses it, because the macro would rename that thing too. When you
add a name to card.h, first check that no card file uses it as an identifier. In converted
files the tool cannot tell a legitimate use from a collision.
