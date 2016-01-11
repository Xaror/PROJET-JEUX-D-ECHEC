package guichess;

/**
 *
 * @author tondeur-h
 */
public class Click {

    int x,y;

    public Click(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public Click(){
        x=-1;
        y=-1;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }



}
