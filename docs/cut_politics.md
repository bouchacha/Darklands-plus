# The cut political campaign

Darklands ships with an almost complete storyline about city politics: a struggle between a
city's rulers and the people who want them gone, which the party can join on either side and
which ends in an arrest, a march on the town hall or the fortress, and a new (or a stronger)
government. Its 19 `$Revol` cards and a dozen helper cards are in the game, written and
coded. None of it ever happens, because the one routine that starts it is never called.

This page charts what exists, what is broken and what is missing. It was checked by switching
the storyline on in a test game (Dortmund, 14 September 1400): the guild masters' job offer,
the interview, the strategy talk and the march on the town hall all run.

## How it was meant to play

1. **Unrest.** Now and then a city has a political season (56 days). Its 15 interest groups
   (the seven guilds, the local merchants, the foreign traders, the Hansa, the Fuggers, the
   Medici, the church, the monastery, the cathedral) each lean towards the rulers or towards
   change, and one guild is where the agitators meet.
2. **Getting involved.** Rumours in the news ("an aristocrat is hiring people to put up
   notices", "someone with a grudge against the lord is hiring") lead to a house in another part
   of the city, or the agitators' guild master takes you to the leader. The leader interviews
   you and you sign on with the rebels or with the government, for pay or for the cause.
3. **The campaign.** Each day you post handbills and thugs from the other side come for you.
   In the guild quarters you sound out the masters, talk them round, dig up the guild leader's
   secret (a heretic, a traitor, a counterfeiter) and use it. You guard friendly houses at
   night and raid the other side's headquarters. The leader tells you how the groups stand.
4. **The decisive day.** When your side is strong enough, the war council sets a day three
   days off: the rebels march, or the guard arrests the ringleaders. The other side can strike
   first, and rumours in the streets warn of it.
5. **The end.** A rally, a march past the guards, a siege of the town hall or the fortress
   (knights may come to relieve it), or an escort of prisoners through an angry crowd. The
   city gets a new government or its ruler grows stronger; the party is rewarded or hunted,
   and travellers elsewhere talk about it afterwards.

## The chart

```mermaid
flowchart TD
    classDef ok fill:#d8f0d0,stroke:#3a7a2a,color:#000
    classDef unreached fill:#fff1c2,stroke:#a07800,color:#000
    classDef stub fill:#ffd9cc,stroke:#b03a10,color:#000
    classDef missing fill:#ffffff,stroke:#b03a10,stroke-dasharray:5 4,color:#000

    SETUP["City politics set up<br/>s039 1462:0558<br/>NEVER CALLED"]:::stub
    NEWS["City news $CityN00<br/>rumours $Speci00"]:::ok
    OTHER["Another part of town $Other00<br/>'the house of ...'"]:::ok
    GUILD["7 guild quarters:<br/>inquire, meet the masters,<br/>sneak in (secrets broken)"]:::ok
    HOUSE["The leader's house<br/>$Revol00 day / $Revol01 night"]:::ok
    JOIN["Job interview $Revol02<br/>choose a side"]:::ok
    PLAN["Strategy talk $Revol03"]:::ok
    WORK["Handbills and thugs $Polit01<br/>night guard $Polit00<br/>clergy $Cleri00"]:::ok
    GOSSIP["Street gossip $Revol04<br/>no way in, 2 options empty"]:::unreached
    EST["Estimate the odds $Situa00<br/>text only"]:::missing
    ARMS["Arms-makers' street $Speci01<br/>text only"]:::missing
    COUNCIL["War council $Consi00<br/>plan options switched off:<br/>'Sorry, this option isn't working yet.'"]:::stub
    DAY["The decisive day<br/>facts 0x15 / 0x16<br/>NEVER SET"]:::missing
    ARREST["Arrest with the sergeant $Revol08"]:::unreached
    RALLY["The rally $Revol09"]:::unreached
    FORGOT["'You forgot!' on the street<br/>$Revol17 / $Revol18"]:::ok
    ENEMY["The other side strikes<br/>$Revol05 rescue / $Revol06 the mob"]:::ok
    ESCORT["Escort the prisoners $Revol10<br/>-> delivered $Revol11"]:::ok
    MARCH["March past the guards $Revol13"]:::ok
    SIEGE["Fortress $Revol14 / town hall $Revol15<br/>relief knights $Revol16, flight $Revol07"]:::ok
    WIN["New government $Revol12"]:::ok
    OUT["Outcome: rewards, reputation<br/>(conflict_outcome)"]:::ok
    AFTER["News afterwards<br/>$Situa01, $Affai00"]:::unreached

    SETUP --> NEWS --> OTHER --> HOUSE --> JOIN
    SETUP --> GUILD --> JOIN
    SETUP -.-> GOSSIP
    SETUP -.-> EST
    SETUP -.-> ARMS
    JOIN --> PLAN --> WORK
    PLAN --> COUNCIL --> DAY
    DAY --> ARREST --> ESCORT
    DAY --> RALLY --> MARCH
    DAY --> FORGOT
    FORGOT --> ESCORT
    FORGOT --> MARCH
    DAY --> ENEMY
    MARCH --> SIEGE --> WIN --> OUT
    ESCORT --> OUT
    ENEMY --> OUT
    OUT --> AFTER
```

Green: written and coded, runs once reached. Yellow: written and coded, but nothing leads to
it. Red: switched off or never called. Dashed: text only, or not written at all.

## What exists

| Piece | State | Where |
|---|---|---|
| The political season of a city (15 groups, the agitators' guild, 56 days) | Coded, **never called** | s039 1462:0558 (0 callers in DARKLAND.EXE) |
| Group arithmetic: each group's side, the two sides' strength | Coded, used by the cards | s039 `loc_side_get/set`, `side1/2_strength`, `side1/2_percent` |
| Rumours in the news, the leader's house by day and by night | Coded, gated on the season | `$CityN00` 0x66, `$Speci00` 0x67, `$Other00` 0x7c, `$Revol00` 0x10, `$Revol01` 0x11 |
| Guild quarters: inquire, meet the masters, the leader's secret, sneak in | Coded, options hidden outside a season | `$Black00/01`, `$Sword00/01`, `$Armor00/01`, `$Bowye00/01`, `$Artif00/01`, `$cloth00/01`, `$jewel00/01` |
| The job interview, the strategy talk | Coded, run | `$Revol02` 0x74, `$Revol03` 0x79 |
| Handbills and thugs, guarding a house at night, swaying the clergy | Coded | `$Polit01` 0x37, `$Polit00` 0x6b, `$Cleri00` 0x6f |
| Street gossip ("government activities", "rebel activities") | Coded but nothing leads to it; those two options do nothing | `$Revol04` 0xbf |
| Estimating the revolt's odds | Text only | `$Situa00` |
| The arms-makers' street to the agitator's home | Text only | `$Speci01` |
| The war council | Text complete; code keeps only "get back to work" | `$Consi00` 0x70 (s072) |
| The decisive day ("the guard arrests today", "the rebels march today") | Read by 15 street cards, **written by nothing** | facts 0x15, 0x16 |
| Being at headquarters on the day: the arrest, the rally | Coded, nothing leads to them | `$Revol08` 0xb7, `$Revol09` 0xb8 |
| Meeting the day on the street, the other side striking | Coded, run (once the day is set) | `$Revol17` 0xbd, `$Revol18` 0xbe, `$Revol05` 0x80, `$Revol06` 0x7b |
| The escort, the march, the sieges, the relief, the flight, the victory | Coded, chained | `$Revol10`–`$Revol16`, `$Revol07` |
| Rewards and reputation at the end; the city marked | Coded | s039 `conflict_outcome` |
| News afterwards, here and in other cities | Texts and some code; wiring unverified | `$Situa01` texts 8–9, `$Affai00` texts 1–2 |

## Bugs found in the never-run code

- The season's own record doubles as the party's side (`loc_fact` queries also match kind-8
  events), and the setup fills it with the city's number instead of 0. The agitators' guild
  master therefore never offers the job (except in city 0).
- No guild leader is ever given a secret: the setup never fills fact 8 (only the cards clear
  it), so asking around never turns up one, sneaking into the leader's home ($Snuck00) never
  finds proof, and "reveal the leader's terrible secret" never appears.
- `$Snuck00` picks one of five ways to find the proof with `% 5` but handles 0, 1, 2, 4 and 5:
  the fourth never does anything, the fifth is never picked (and compares a roll with a member's
  number instead of the skill).
- `$Revol13` sets the bribe's option to "a saint can be called" (`opt_flag_1 = saints_usable()`,
  meant for the saint's option), so the bribe can be paid without the money. (`$Revol14` has
  the same kind of mistake with the rope, already corrected by the `cardopts` fix.)
- `$Revol02` text 7 and `$Revol18` show "$VarString1, the $VarString2" unfilled (the town hall or
  fortress name is not set before them).
- `$Revol03`'s report on the clergy reuses the merchants' attitude for the citizens and the
  Medici's for the bishop.
- `conflict_outcome` works out a chance of success from the two sides' strength and then
  throws it away.
- `$Cleri00` adds to the "fact" numbered by `party_x`, which the cards that lead to it leave at
  0 or 1, so a success can change the party's own side.
- Street cards: a night/day chance inside `if (side == -1) { if (side == 1) ... }` can never be
  used.
- Typos in the texts: "virutous", "the the guildsmen".

## What completing it takes

1. **Switch it on (small).** Start a season now and then in a city (for example: on arrival,
   if none is running anywhere, with a small chance), with the side set to 0. Optional, off by
   default, in Settings.
2. **The decisive day (medium).** Code the war council's options from its texts: the tally,
   "review the plan", "put the plan into effect, three days from now". Write facts 0x15/0x16
   for that day. Let the other side set its own day when it is stronger, with the warnings of
   `$Revol04` texts 4–9 in the street gossip. Start `$Revol08`/`$Revol09` when the party is
   at headquarters at that hour; the street cards already start `$Revol17/18` and `$Revol05/06`
   otherwise. End the season afterwards.
3. **The missing small cards (small).** A way into `$Revol04`; `$Situa00` and `$Speci01` written
   as cards from their texts, or left out.
4. **The bugs above, and playtesting (large).** The secrets need the setup to give some guild
   leaders one. Then: both sides, both outcomes, being there or not, leaving the city
   mid-campaign, saving and loading in between, the season running out. None of this code has
   ever run for players, so more will turn up.

## Trying it today

The test command `upolit:` (port/runtime/testcmds.c) starts a season in the current city. Set
the side to 0 with `ucall:29,0e76,2e74,-2,LOC,28,0,0,2,0` (`event_update`), then go to the
guild named by fact 0x17 (`uevents:28`: type 23, a bit: 1 blacksmiths, 2 swordsmiths, 8 bowyers,
0x10 armourers, 0x20 artisans, 0x40 clothmakers, 0x80 jewellers) and ask to meet the masters.
`ucall:29,0e76,2c4e,-2,LOC,90,28,22,0,0,24,0,2` (`event_new`) makes "the rebels march today"
(type 21 for the arrest); the next street card starts the day.

## Reference: the season's facts (tag 2, at the city)

| Fact | Meaning |
|---|---|
| 0 | The party's side: -1 rebels, 1 government, 0 none (the season record itself) |
| 1, 2 | The groups (bits) leaning to side -1, to side 1 |
| 3, 4, 5 | Per guild (bits): asked about politics, met the masters, tried to sneak in |
| 8 | Per guild (bits): its leader has a secret (never set: see the bugs) |
| 9, 0xa | Per guild (bits): the secret known, its proof found |
| 0x14 | Houses guarded at night |
| 0x17 | The agitators' guild (one bit) |
| 0x19, 0x1a | The two hiring rumours heard |
| 0x1b–0x22 | Visits to the leader's house (by day, by night; how they went) |
| 0x15, 0x16 | The guard arrests today / the rebels march today (never written) |
| 0x24, 0x25 | The government's strike / the rebels' march has taken place |
| 0x26 | A day of siege waited out |
| 0x23 | A random 0–99 (read by nothing) |

Group bits: 1 local merchants, 2 blacksmiths, 4 swordsmiths, 8 bowyers, 0x10 armourers, 0x20
artisans, 0x40 clothmakers, 0x80 jewellers, 0x400 and 0x800 the city church and the monastery,
0x1000 foreign traders, 0x2000 the Hansa, 0x4000 the Fuggers, 0x8000 the Medici, 0x200 (no card
names it consistently: the people or the cathedral).
