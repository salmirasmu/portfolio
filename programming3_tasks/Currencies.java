/**
 *
 * @author Rasmus Salmi
 * 
 * Tämä luokka toteuttaa yksinkertaisen valuuttamuuntimen.
 * Komennot:
 * rate AAA x: kirjaa muistiin, että 1 euro on x valuutan
 * 	       AAA rahayksikköä.
 * convert x AAA: tulostaa kuinka monta euroa x valuutan
 *                AAA rahayksikköä vastaa.
 * rates: tulostaa kirjatut valuuttakurssit.
 * quit: lopettaa ohjelman toiminnan. 
 */

import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.TreeMap;

public class Currencies {
    /**
     * @param args the command line arguments
     * @throws java.io.IOException
     */
    public static void main(String[] args) throws IOException {
        TreeMap<String, Double> currencies = new TreeMap<>();
        BufferedReader user = new BufferedReader(new InputStreamReader(System.in));
        while (true){
            System.out.print("Enter command: ");
            String input = user.readLine();
            System.out.print(input);
            String[] parameters = input.split("\\s+");
            System.out.print("\n");
            if (parameters[0].equalsIgnoreCase("rate"))
            {
                Double currency = Double.parseDouble(parameters[2]);
                currencies.put(parameters[1].toUpperCase(), currency);
                System.out.format("Stored the rate 1 EUR = %.3f %s\n",
                        currency, parameters[1].toUpperCase());
            }
            else if (parameters[0].equalsIgnoreCase("convert"))
            {
                if (currencies.get(parameters[2]) == null)
                {
                    System.out.print("No rate for " + parameters[2] +" has been stored!\n");
                }
                else
                {
                    Double ammount = Double.parseDouble(parameters[1]);
                    Double value = ammount/currencies.get(parameters[2]);
                    System.out.format("%.3f " + parameters[2] + " = %.3f EUR\n"  , ammount, value);
                }
            }
            else if (parameters[0].equalsIgnoreCase("rates"))
            {
                System.out.print("Stored euro rates:\n");
                for (String key : currencies.keySet())
                {
                    System.out.format("  " + key + " %.3f\n", currencies.get(key));
                }
            }
            else if (parameters[0].equalsIgnoreCase("quit"))
            {
                System.out.print("Quit-command received, exiting...\n");
                break;
            }
            else{
                System.out.print("Unknown or illegal command!\n");
            }
        }
    }
} 