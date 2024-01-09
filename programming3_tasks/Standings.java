/**
 * Rasmus Salmi
 *
 * This class maintains information about teams in a tournament.
 * Team earns 3 points for each win, 1 point for each tie and 0 points
 * for each loss.
 */

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.TreeMap;
import java.util.ArrayList;
import java.util.Map;
import java.util.Collections;


public class Standings {

    public static class Team 
    {
        private String name;
        private int wins;
        private int ties;
        private int loses;
        private int scored;
        private int allowed;
        private int points;
        private int gd;
        
        private int gd()
        {
            return scored-allowed;
        }
        public Team(String name) {
            this.name = name;
        }
        
        public String getName(){
            return name;
        }
       
        public int getWins() {
            return wins;
        }

        public int getTies() {
            return ties;
        }

        public int getLosses() {
            return loses;
        }

        public int getScored() {
            return scored;
        }

        public int getAllowed() {
            return allowed;
        }

        public int getPoints() {
            return points;
        }
    }
    
    private TreeMap<String, Team> standings;
    private ArrayList<Team> teams;
    private ArrayList<Map.Entry<String, Team>> sorted;
    private int max_lenght = 0;
    
    private void init_standings(String filename) throws IOException
    {
        try(var input = new BufferedReader(new FileReader(filename)))
        {
            String line = null;
            while ((line = input.readLine()) != null)
            {
                String[] parts = line.split("\\t");
                String[] goals = parts[1].split("-");
                Team teamA = standings.computeIfAbsent(parts[0], tn -> new Team(parts[0]));
                Team teamB = standings.computeIfAbsent(parts[2], tn -> new Team(parts[2]));
                int goalsA = Integer.parseInt(goals[0]);
                int goalsB = Integer.parseInt(goals[1]);
                
                add_results(goalsA, goalsB, teamA, teamB);
            }
        }
    }
    
    public Standings(String filename) throws IOException {
        standings = new TreeMap<>();
        init_standings(filename);
    }

    public Standings() {
        standings = new TreeMap<>();
    }
    
    public void readMatchData(String filename) throws IOException
    {
        init_standings(filename);
    }
    
    public void addMatchResult(String teamNameA, int goalsA, int goalsB, String teamNameB)
    {
        Team teamA = standings.computeIfAbsent(teamNameA, tn -> new Team(teamNameA));
        Team teamB = standings.computeIfAbsent(teamNameB, tn -> new Team(teamNameB));
        add_results(goalsA, goalsB, teamA, teamB);
    }
    
    void add_results(int a, int b, Team teamA, Team teamB)
    {
        int lengthA = teamA.name.length();
        int lengthB = teamB.name.length();
        
        //laskee pismimman joukkueen nimen
        if (lengthA > max_lenght )
        {
            max_lenght = lengthA;
        }
        if (lengthB > max_lenght )
        {
            max_lenght = lengthB;
        }
        teamA.scored += a;
        teamA.allowed += b;
        teamB.scored += b;
        teamB.allowed += a;

        if (a > b)
        {
            teamA.wins += 1;
            teamA.points += 3;
            teamB.loses += 1;
        }
        if (a < b)
        {
            teamB.wins += 1;
            teamB.points += 3;
            teamA.loses += 1;
        }
        if (a == b)
        {
            teamA.ties += 1;
            teamA.points += 1;
            teamB.ties += 1;
            teamB.points += 1;
        }
    }
    public ArrayList<Team> getTeams()
    {
        sorted = new ArrayList<>(standings.entrySet());
        Collections.sort(sorted, (a, b) -> {
            if (a.getValue().points != b.getValue().points)
            {
                return b.getValue().points - a.getValue().points;
            }
            if (a.getValue().gd() != b.getValue().gd())
            {
                return b.getValue().gd() - a.getValue().gd();
            }
            if (a.getValue().scored != b.getValue().scored)
            {
                return b.getValue().scored - a.getValue().scored;
            }
            else
            {
                return a.getKey().compareTo(b.getKey());
            }
        });
        teams = new ArrayList<Team>();
        for (Map.Entry<String, Team> team : sorted)
        {
            teams.add(team.getValue());
        }
        return teams;
    }
    
    public void printStandings()
    {
        getTeams();
        
        for (Team team : teams)
        {
            int game_played = team.wins+team.loses+team.ties; 
            String gdr = Integer.toString(team.scored)+"-"+Integer.toString(team.allowed);
            
            System.out.format("%s%"+ (max_lenght+4-team.name.length()) + "d%4d%4d%4d%7s%4d",
                    team.name, game_played, team.wins, team.ties,team.loses, gdr, team.points);
            System.out.print("\n");
        }
    }
}    


