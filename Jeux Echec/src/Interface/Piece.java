/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Interface;

import javafx.scene.image.ImageView;

/**
 *
 * @author Bloody
 */
public class Piece {
    private String nom;
    private ImageView img;
    private String id;
    private String type;
    
    
    
    public Piece ( String nom, ImageView img, String type){
        this.nom= nom;
        this.img= img;
        this.type=type;
    }
    public String getnom(){
        return nom;
    }
    public ImageView getimg(){
        return img;
    }
    public String getid(){
        return id;
    }
    public void setid( String id){
        this.id=id;
    }
    public String gettype(){
        return type;
    }
    public void settype( String type){
        this.type=type;
    }
            
}
