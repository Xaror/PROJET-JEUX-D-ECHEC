package guichess;

/**
 *
 * @author tondeur-h
 */
public class Pieces {

public static final String VIDE="none";
public static final String TOURB="tourb";
public static final String CAVB="cavb";
public static final String FOUB="foub";
public static final String DAMEB="dameb";
public static final String ROIB="roib";
public static final String PIONB="pionb";

public static final String TOURN="tourn";
public static final String CAVN="cavn";
public static final String FOUN="foun";
public static final String DAMEN="damen";
public static final String ROIN="roin";
public static final String PIONN="pionn";

public static final int BLANCHE=0;
public static final int NOIRE=1;

String nom;


    public String getNom() {
        return nom;
    }

    public void setNom(String nom) {
        this.nom = nom;
    }

    public Pieces(String nom) {
    this.nom=nom;
    }



}
