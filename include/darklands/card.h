/* darklands/card.h - readable names for the card-script API (s044..s135).
 *
 * Macros ONLY: every name expands to the raw linker name (D_xxxx = DGROUP offset,
 * F_ssss_xxxx = far data, sub_<img>_<frame>_<off> = original function), so the compiled code
 * is unchanged.  Each card file keeps its own extern/prototype lines (their order matters
 * to MSC 6.00A), written with these names.  "?" in a comment = meaning not fully certain.
 * See docs/CARDS.md.  tools/cardnames.py converts a card file to these names.
 *
 * Conventions: "who" is a party member index 0..4; -1 = the active member (leader),
 * -2 = every present member.  "loc" is a location index (cities are 0..0x5b).
 */

/* ---- card engine (s043 150b): next card, text, option menu ---------------------- */
#define next_card            D_e48a   /* card the engine runs next (the script sets it) */
#define card_name            D_e3d4   /* name buffer of the current card ("$CESde00") */
#define pic_buf              D_e3e3   /* picture name buffer (passed to card_load) */
#define pic_buf2             D_e95f   /* picture name buffer (passed to card_load_near) */
#define pic_names            F_2caf_0000 /* far table of picture names ("common", "xmob", ...) */
#define card_load            sub_s043_150b_000a /* (name, char far *pic): show picture, load msgfiles\<name>.msg */
#define card_load_near       sub_s043_150b_0110 /* (name, char *pic): same with a near picture name, no log */
#define card_text            sub_s043_150b_03b0 /* (block, how): show text block, wait for a key */
#define card_options         sub_s043_150b_049a /* (block): show the option-list text block (no wait) */
#define card_menu            sub_s043_150b_1142 /* (win, win2): run the option menu, returns option index */
#define saints_usable        sub_s043_150b_168c /* 1 if an able member knows a saint of card_saints[], else 2 */
#define items_usable         sub_s043_150b_16e8 /* 1 if an able member has an item of card_items[], else 2 */
#define card_window          D_017c   /* card text window (first argument of card_menu) */
#define card_window2         D_0154   /* second argument of card_menu (?) */
#define options_block        D_ee53   /* a block some scripts keep for card_options (s083); others pass a number, so it may be an earlier card's: see shown_block */
#define shown_block          D_e959   /* the text block on show: set by card_text and card_options (while a menu runs: its list) */
#define msg_buf              D_9085   /* message text: option handlers strcpy their result text here */

/* option tables, filled by the script before card_menu */
#define opt_on               D_e99c   /* char[20]: option enabled flags */
#define opt_on_1             D_e99d
#define opt_on_2             D_e99e
#define opt_on_3             D_e99f
#define opt_on_4             D_e9a0
#define opt_on_5             D_e9a1
#define opt_on_6             D_e9a2
#define opt_on_7             D_e9a3
#define opt_on_8             D_e9a4
#define opt_on_9             D_e9a5
#define opt_fn               D_ea14   /* struct res (far *[20])(): option handlers (chance/text) */
#define opt_fn_1             D_ea18
#define opt_fn_2             D_ea1c
#define opt_fn_3             D_ea20
#define opt_fn_4             D_ea24
#define opt_fn_5             D_ea28
#define opt_fn_6             D_ea2c
#define opt_fn_7             D_ea30
#define opt_fn_8             D_ea34
#define opt_fn_9             D_ea38
#define opt_flag_0           D_ee76   /* per-option state the script copies into opt_on[0] */
#define opt_flag_1           D_ee78
#define opt_flag_2           D_ee7a
#define opt_flag_3           D_ee7c
#define opt_flag_4           D_ee7e
#define opt_flag_5           D_ee80
#define opt_flag_6           D_ee82
#define opt_flag_7           D_ee84
#define opt_flag_8           D_ee86
#define opt_flag_9           D_ee88   /* ... opt_on[9] */

/* item / saint pop-up lists (skill-check boxes of option lines) */
#define card_items           D_ee43   /* int[4]: item codes usable here (potions ...), -1 = none */
#define card_items_1         D_ee45
#define card_items_2         D_ee47
#define card_items_3         D_ee49
#define card_saints          D_ee4b   /* int[4]: saints that can be invoked here, -1 = none */
#define card_saints_1        D_ee4d
#define card_saints_2        D_ee4f
#define card_saints_3        D_ee51
#define item_fn              D_ec00   /* struct res (far *[5][4])(): handler per member and card_items slot */
#define saint_fn             D_ecf6   /* struct res (far *[5][4])(): handler per member and card_saints slot */
#define menu_member          D_ee6c   /* member chosen in the pop-up box */
#define menu_slot            D_ee6a   /* item/saint slot chosen in the pop-up box */
#define menu_popup           D_ee6e   /* 1: the choice came from the pop-up box (not an option line) */

/* ---- $variables of the card text (expanded by s043 150b 0886) ------------------- */
#define var_chosen1          D_ee8c   /* $ChosenOneName, $He/$he/$his/$him/$himself */
#define var_chosen2          D_ee8e   /* $ChosenTwoName */
#define var_chosen3          D_ee90   /* $ChosenThreeName */
#define var_chosen4          D_ee92   /* $ChosenFourName */
#define var_chosen5          D_ee94   /* $ChosenFiveName */
#define var_money1           D_ee9c   /* struct money: $Money1 */
#define var_money2           D_eea2   /* $Money2 */
#define var_money3           D_eea8   /* $Money3 */
#define var_money4           D_eeae   /* $Money4 */
#define var_money5           D_ed46   /* $Money5 */
#define var_number1          D_ed4c   /* $Number1 */
#define var_number2          D_ed50   /* $Number2 */
#define var_text1            D_ed6d   /* char[40]: $Text1 */
#define var_text2            D_ed95   /* $Text2 */
#define var_text3            D_edbd   /* $Text3 */
#define var_text4            D_ede5   /* $Text4 */
#define var_item             D_ed5c   /* item index for $ItemName */
#define var_loc              D_e89b   /* location for $LocName (also $NearestCity, $Direction) */
#define var_loc2             D_ed54   /* location for $LocName2 / $Direction2 */
#define var_support0         D_ed5e   /* $Support .. $Support6: indices into the support names */
#define var_support1         D_ed60
#define var_support2         D_ed62
#define var_support3         D_ed64
#define var_support4         D_ed66
#define var_support5         D_ed68
#define var_support6         D_ed6a
#define var_named            F_2e38_1a96 /* char far [5][30]: $NamedOneName..$NamedFiveName */

/* shared card strings (defined in s043 1462) */
#define str_screwed_up       D_4ec4   /* "screwed up." */
#define str_sleep_dawn       D_4ed0   /* "You'll sleep until dawn." */
#define str_wait_night       D_4eea   /* "You'll wait for night." */
#define str_no_risk          D_4f02   /* "No risk." */
#define str_select_potion    D_4f0c   /* "Select a person and a potion." */
#define str_select_saint     D_4f2a   /* "Select a person and a saint." */
#define str_success_assured  D_4f48   /* "Success is assured." */
#define str_saint_xxx        D_4f5c   /* "XXX Saint % success msg XXX." */
#define str_potion_works     D_4f7a   /* "Of course the potion will work!" */
#define str_unknown          D_4f9a   /* "You can always encounter the unknown" */
#define str_enter_combat     D_4fc0   /* "Enter Combat" */
#define quality_words        D_4fce   /* char *[7]: "very poor" .. "superb" */

/* ---- party, world, time --------------------------------------------------------- */
#define party                D_9c69   /* party member records, 5 x 128 bytes (+0 status: 1 = able) */
#define party_size           D_a67e   /* number of party members */
#define leader               D_907b   /* active member (the leader) */
#define party_money          D_906b   /* struct money {florins, groschen, pfennigs}: the purse */
#define party_seed           D_9c4a   /* party random seed */
#define party_x              D_e3d0   /* party map position */
#define party_y              D_e3d2
#define best_member          D_991d   /* member found by the last party_min/max query */
#define best_value           D_99e6   /* skill value found by the last party skill query */
#define best_skill           D_99eb   /* skill index found by party_best_skill_* */
#define cur_loc              D_907e   /* current location (< 0x5c: city), -1 = wilderness */
#define city_rec             D_9785   /* char far *: record of the current city (darkland.cty) */
#define city_desc            D_978b   /* description of the current city (darkland.dsc) */
#define locations            F_2e38_10be /* far pointers to the 0x19e location records */
#define events               F_2e38_1b2c /* far pointers to the 300 timed-event records */
#define item_defs            F_2e38_0890 /* far pointers to the item definitions */
#define last_event           D_e3ce   /* index of the event created/extended last */
#define cur_rep              D_ee74   /* the script's copy of cur_loc's reputation (loc_query(cur_loc, 0)) */
#define saved_card           D_e7d8   /* card number kept for later, saved with the game (? return card) */
#define saved_card2          D_e896   /* another one (?) */
#define cur_hour             D_00e0   /* 0-23 */
#define cur_day              D_00e2   /* day of month, 0-based */
#define cur_month            D_00e4   /* 0-11 */
#define cur_year             D_00e6
#define prev_screen          D_a88d   /* screen/card code before the current one (? "go back") */
#define battle_mode          D_a766   /* battle parameters used when a card returns -2..-6 (?) */
#define battle_enemy         D_a768   /* (?) enemy/encounter type */
#define battle_seed          D_a76a   /* encounter seed: rand_mod(1000) or party_seed + cur_loc */
#define battle_result        D_a893   /* outcome of the last battle (return of run_battle) */

/* ---- random numbers, mouse (root) ----------------------------------------------- */
#define rand_mod             sub_root_0000_0c70 /* (n): rand() % n, 0..n-1 (n < 2 counts as 2) */
#define mouse_show           sub_root_047c_1293 /* show the mouse cursor */
#define mouse_hide           sub_root_047c_12c3 /* hide the mouse cursor */

/* ---- rules: numbers, money, time, text (s029 1367) ------------------------------ */
#define clamp99              sub_s029_1367_000a /* (v): 0..99 */
#define clamp100             sub_s029_1367_0028 /* (v): 0..100 */
#define clamp_pm100          sub_s029_1367_0046 /* (v): -100..100 */
#define clamp_range          sub_s029_1367_0066 /* (lo, hi, v) */
#define imax                 sub_s029_1367_0084 /* (a, b) */
#define imin                 sub_s029_1367_0094 /* (a, b) */
#define money_set            sub_s029_1367_00a4 /* (m, fl, gr, pf), normalised (20 gr = 1 fl, 12 pf = 1 gr) */
#define money_pfennigs       sub_s029_1367_00f2 /* long (m): value in pfennigs */
#define money_add_fgp        sub_s029_1367_0130 /* (m, fl, gr, pf): m += */
#define money_sub_fgp        sub_s029_1367_0180 /* (m, fl, gr, pf): m -= */
#define money_copy           sub_s029_1367_01ce /* (dst, src) */
#define money_add            sub_s029_1367_01ec /* (a, b): a += b */
#define money_sub            sub_s029_1367_023e /* (a, b): a -= b */
#define money_mul            sub_s029_1367_0290 /* (dst, unit, n): dst = unit * n */
#define money_cmp            sub_s029_1367_02d4 /* (a, b): 1 a > b, -1 a < b, 0 equal */
#define money_format         sub_s029_1367_0376 /* (m, buf): "3 florins, 2 groschen and 1 pfennig" */
#define money_per_member_pf  sub_s029_1367_054e /* (total, pf): total += pf for each able member */
#define money_per_member     sub_s029_1367_0594 /* (total, cost): total += cost for each able member */
#define advance_hours        sub_s029_1367_05c8 /* (n): let n hours pass (hourly/monthly updates) */
#define advance_days         sub_s029_1367_0680 /* (n): let n days pass */
#define wait_until_hour      sub_s029_1367_0716 /* (hour) */
#define is_daylight          sub_s029_1367_072a /* 1 from 5:00 to 18:59 */
#define get_hour             sub_s029_1367_0740
#define hour_band            sub_s029_1367_0744 /* (h): time-of-day band of an hour */
#define cur_band             sub_s029_1367_0796 /* time-of-day band now */
#define cur_band1            sub_s029_1367_07ee /* the same numbered 1..7, 0 for 21-23 */
#define hours_until          sub_s029_1367_086a /* (hour): hours from now until that hour */
#define message_box          sub_s029_1367_0c30 /* (msg): show a message box, wait for a key */
#define chance_text          sub_s029_1367_0c52 /* (buf, lo, hi): append "Chance of success is X% to Y%" */
#define do_nothing           sub_s029_1367_05c6 /* empty function */
#define random_name          sub_s029_1367_0db4 /* (seed, kind, slot): random personal name into var_named[slot] */

/* ---- rules: party members (s029 0e76); record: +0x49 attributes, +0x50 their
 *      maxima, +0x57 skills; results of party queries also in best_member/best_value */
#define member_name          sub_s029_0e76_0000 /* (who, buf) */
#define member_short_name    sub_s029_0e76_003e /* (who, buf) */
#define member_age           sub_s029_0e76_00a2 /* (who): years */
#define member_skill         sub_s029_0e76_01a0 /* (who, k) */
#define member_attr          sub_s029_0e76_05ee /* (who, k): current attribute */
#define member_attr_max      sub_s029_0e76_0b64 /* (who, k): attribute maximum */
#define add_skill            sub_s029_0e76_02ea /* (who, k, d): clamp 0..99, reports the change */
#define set_skill            sub_s029_0e76_02b8 /* (who, k, v) */
#define add_attr             sub_s029_0e76_0a72 /* (who, k, d): clamp 1..99 and to the maximum */
#define set_attr             sub_s029_0e76_0988 /* (who, k, v) */
#define add_attr_max         sub_s029_0e76_0bb6 /* (who, k, d): clamp 1..99 */
#define set_attr_max         sub_s029_0e76_0b84 /* (who, k, v) */
#define party_best_skill_lo  sub_s029_0e76_021e /* best of skills 0-3 over present members */
#define party_best_skill_hi  sub_s029_0e76_026a /* best of skills 4-6 over present members */
#define member_best_skill    sub_s029_0e76_01c0 /* (who): highest of the 7 skills +0x57..+0x5d */
#define party_min_skill      sub_s029_0e76_1396 /* (k): lowest skill k of the able members */
#define party_min_skill2     sub_s029_0e76_13de /* (a, b): lowest of skills a and b */
#define party_min_skill_sum  sub_s029_0e76_1446 /* (a, b): lowest a + b */
#define party_max_skill      sub_s029_0e76_14a4 /* (k): highest skill k of the able members */
#define party_max_skill2     sub_s029_0e76_153a /* (a, b): highest of skills a and b */
#define party_max_skill_sum  sub_s029_0e76_15a2 /* (a, b): highest a + b */
#define party_avg_skill      sub_s029_0e76_1600 /* (k) */
#define party_min_attr       sub_s029_0e76_164a /* (k) */
#define party_max_attr       sub_s029_0e76_16fe /* (k) */
#define party_max_attr_sum   sub_s029_0e76_179c /* (a, b) */
#define party_avg_attr       sub_s029_0e76_1800 /* (k) */
#define member_eff_attr2     sub_s029_0e76_06de /* (who): attribute 2 scaled by fatigue */
#define party_avg_eff_attr2  sub_s029_0e76_060e
#define party_min_eff_attr2  sub_s029_0e76_0656
#define party_max_eff_attr2  sub_s029_0e76_0690
#define party_max_eff_attr2_bonus sub_s029_0e76_072e /* + best quality of items with flag 0x2000 */
#define party_min_eff_attr2_bonus sub_s029_0e76_088e
#define count_items          sub_s029_0e76_0c76 /* (who, code): items of type code carried */
#define count_items_flags    sub_s029_0e76_0dd8 /* (who, long mask): items whose type flags match */
#define best_item_quality    sub_s029_0e76_0f54 /* (who, long mask) */
#define has_item             sub_s029_0e76_118e /* (who, code): 1 carried, 2 known, 0 no */
#define member_bit           sub_s029_0e76_1260 /* (who, bit): knowledge bit (saints known ...) */
#define set_member_bit       sub_s029_0e76_1302 /* (who, bit) */
#define party_get            sub_s029_0e76_1326 /* (what) */
#define party_set            sub_s029_0e76_1382 /* (what, v) */
#define improve_roll         sub_s029_0e76_18a8 /* (v, lo, hi): lo + rand(hi - lo) with chance |100 - v|%, else lo */
#define party_count_able     sub_s029_0e76_007c /* members present and able */
#define party_all_ok         sub_s029_0e76_1972 /* 0 if a member has status 2 or 3, else 1 */
#define saint_chance_lo      sub_s029_0e76_20c6 /* (who, s): lower chance of invoking saint s */
#define saint_chance_hi      sub_s029_0e76_2116 /* (who, s): upper chance */
#define invoke_saint         sub_s029_0e76_2180 /* (who, s): pray, then redraw the card */
#define run_battle           sub_s029_0e76_2278 /* (enemy?, seed, ...): fight, returns battle_result */
#define party_report_away    sub_s029_0e76_23e2 /* members with status 5: message, returns count */
#define party_drop_away      sub_s029_0e76_2434 /* members with status 5 leave the party */
#define party_rejoin_away    sub_s029_0e76_251a /* members with status 5 return (status 1) */
#define add_member           sub_s029_0e76_4650 /* (0, name, shortname, pic, ...): new party member */
#define raise_attr_temp      sub_s029_0e76_4c04 /* (who, k, d, h): temporary attribute boost */
#define lower_skill_temp     sub_s029_0e76_4b08 /* (who, k, d, h): temporary skill change */

/* ---- rules: locations (s029 0e76) ----------------------------------------------- */
#define loc_reputation       sub_s029_0e76_199c /* (loc) */
#define loc_reputation_adj   sub_s029_0e76_19d0 /* (loc, a, b): random reputation change */
#define cur_loc_query        sub_s029_0e76_1a7e /* (what): loc_query(cur_loc, what) */
#define loc_query            sub_s029_0e76_1a8e /* (loc, what): 0 reputation, 1 its band, 2 city size, 3.. city data */
#define loc_set              sub_s029_0e76_1dfe /* (loc, what, v): counterpart of loc_query */
#define name_to_slot         sub_s029_0e76_1fb8 /* (kind, idx, slot): copy a name into var_named[slot] */
#define load_city            sub_s029_0e76_4e68 /* (city): read darkland.cty / .dsc records */

/* ---- rules: timed events (s029 0e76); event: kind, type, owner f1a (-2 party),
 *      location f1c, value f1e, f28 group/tag; queries count only started events ---- */
#define event_new            sub_s029_0e76_2c4e /* (owner, loc, c, kind, type, f, g, hours, x, tag): 9999 h = forever */
#define event_renew          sub_s029_0e76_2d5c /* find or create, extend by hours */
#define event_update         sub_s029_0e76_2e74 /* (owner, loc, kind, type, v, tag, op): value op 0 set 1 add 2 and 3 or */
#define event_free           sub_s029_0e76_30fe /* (i) */
#define event_here           sub_s029_0e76_2930 /* (kind, hours): event at cur_loc */
#define event_here_type      sub_s029_0e76_2a32 /* (kind, type, hours): event at cur_loc */
#define event_here_extend    sub_s029_0e76_2b36 /* (kind, hours): extend or create */
#define event_exists         sub_s029_0e76_32aa /* (kind) */
#define event_started        sub_s029_0e76_32ce /* (kind): the first event of kind has started */
#define event_count          sub_s029_0e76_37b8 /* (kind) */
#define event_at             sub_s029_0e76_3878 /* (kind, loc) */
#define event_type           sub_s029_0e76_38d2 /* (kind, type) */
#define event_type_at        sub_s029_0e76_392c /* (kind, type, loc) */
#define event_value          sub_s029_0e76_39f0 /* (kind): value of the first active one */
#define event_value_at       sub_s029_0e76_3a20 /* (kind, loc) */
#define event_value_type     sub_s029_0e76_3a86 /* (kind, type) */
#define event_value_type_at  sub_s029_0e76_3aec /* (kind, type, loc) */
#define event_find           sub_s029_0e76_3b62 /* (kind, type, loc, owner, tag, f2a): index or -1 (-1 = any) */
#define event_find_loc       sub_s029_0e76_3c28 /* (kind, type, owner, tag, f2a): its location or -1 */
#define event_remove         sub_s029_0e76_3cde /* (kind, type, loc, owner, tag) */
#define loc_fact_any         sub_s029_0e76_35aa /* (tag, type): kind-0x1c event exists */
#define loc_fact             sub_s029_0e76_360c /* (tag, type, loc) */
#define loc_fact_value_any   sub_s029_0e76_36d4 /* (tag, type): its value */
#define loc_fact_value       sub_s029_0e76_3742 /* (tag, type, loc): its value */
#define loc_facts_clear      sub_s029_0e76_3d92 /* (tag, loc): remove kind-0x1c events (-1 = any) */
#define ev8_exists           sub_s029_0e76_32fe /* (tag): kind-8 event */
#define ev8_at               sub_s029_0e76_3470 /* (tag, loc) */
#define ev8_at_owner         sub_s029_0e76_34d2 /* (tag, owner, loc) */
#define ev8_value            sub_s029_0e76_3676 /* (tag) */
#define ev8_owner_value      sub_s029_0e76_353e /* (tag, owner, value): kind-8 event with that value */
#define ev8_resolve          sub_s029_0e76_3e06 /* (tag, loc): turn kind-8 events into kind 0x24 */
#define ev24_exists          sub_s029_0e76_3350 /* (tag): kind-0x24 event */
#define ev24_at              sub_s029_0e76_33a2 /* (tag, loc) */
#define ev24_at_owner        sub_s029_0e76_3404 /* (tag, owner, loc) */

/* ---- rules toolkit s039 1462, reached through overlay vectors (vec_xxxx) -------- */
#define practice_skill       vec_1f63 /* (who, k, mode, pct, lvl): chance that skill k improves */
#define party_lore_level     vec_1c1b /* 1..10 from the party's best/average skills and size */
#define injure               vec_2143 /* (who, a, b, mult): damage (attribute 1 and 0 lose points) */
#define party_present_count  vec_2161 /* number of present party members */
#define loc_hostile          vec_20f3 /* 1 if the current location is hostile (reputation, flags) */
#define loc_side_get         vec_1f59 /* (bit): three-state flag of cur_loc: -1, 0, 1 */
#define loc_side_set         vec_1f4f /* (bit, mode): set it (-1, 0, 1) */
#define side1_percent        vec_1f45 /* strength of side -1 in percent of side 1 */
#define side2_percent        vec_1f3b /* strength of side 1 in percent of side -1 */
#define side1_strength       vec_2111 /* strength of the -1 side (flags of cur_loc) */
#define side2_strength       vec_2189 /* strength of the 1 side */
#define conflict_outcome     vec_217f /* (a, b, c): outcome of a fight at cur_loc */
#define give_reward          vec_1f31 /* (k): reward of size k (0, 3, 10, 25, 64, 200 florins scaled), returns it */
#define event_loc_of         vec_211b /* (i): location (f1c) of event i */
#define event_f2e_of         vec_2125 /* (i): field f2e of event i (item, ...) */
#define event_type_of        vec_2139 /* (i): type of event i */

/* ---- overland map (s030) and inventory (s031) through vectors ------------------- */
#define nearest_city         vec_1fa9 /* (x, y): nearest city to a map position */
#define nearest_loc_type     vec_1f77 /* (x, y, type): nearest location of a map type */
#define lose_items           vec_1eb9 /* (who): lose heavy stacks, light ones by chance (-2: also the money) */
#define take_item            vec_2085 /* (who, code): remove one item, -1 if none */
#define add_item             vec_2067 /* (who, code): add one item of default quality */
#define add_items            vec_2053 /* (who, code, n) */
#define add_item_rec         vec_1f6d /* (who, struct item): add or stack */
#define item_quality         vec_205d /* (who, code): best quality of item code */
#define keep_items_pct       vec_2021 /* (who, pct): each stack survives with pct% */
#define remove_items_flags   vec_202b /* (who, long mask): remove items whose type flags match */
#define remove_item_stack    vec_208f /* (who, code): remove the first stack of code */
#define equipped_item        vec_1e9b /* (who, slot): item code in an equipment slot, -1 none */
#define item_quality_flags   vec_2071 /* (who, long flags) */
