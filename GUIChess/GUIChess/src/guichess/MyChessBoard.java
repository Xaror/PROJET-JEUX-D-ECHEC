
package guichess;

import java.util.ArrayList;

/**
 *
 * @author tondeur-h
 */
public class MyChessBoard {

    private ArrayList<Pieces> chessB;

    public MyChessBoard() {
    chessB=new ArrayList<Pieces>(64);
            for (int casex=0;casex<64;casex++){
                chessB.add(casex,new Pieces(Pieces.VIDE));
            }
    }


    /**
     * retourne la couleur de la case
     * @param x
     * @param y
     * @return
     */
    public int couleurCase(int x,int y){
        //si y est pair et x est pair => blanc
        //si y est impair et x est impair => blanc
        if ((((x%2)==0) && ((y%2)==0)) || (((x%2)!=0) && ((y%2)!=0)) )
            return Pieces.BLANCHE; else return Pieces.NOIRE;
    }


    /**
     * affecte une piece a une case
     * @param x
     * @param y
     * @param piece
     */
    public void setPieces(int x,int y,Pieces piece){
        chessB.set((8*y)+x, piece);
    }


    /**
     * Supprime une piece d'une case
     * @param x
     * @param y
     */
    public void delPieces(int x,int y){
        chessB.set((8*y)+x,new Pieces(Pieces.VIDE));
    }

    /**
     * vider l'échiquier
     */
    public void clearPieces(){
        for (int casex=0;casex<64;casex++){
                chessB.add(casex,new Pieces(Pieces.VIDE));
            }
    }

    /**
     * Recuperer la pieces de la case...
     * @param x
     * @param y
     * @return
     */
    public Pieces getPieces(int x,int y){
        return chessB.get((8*y)+x);
    }


}
