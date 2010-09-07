import java.applet.Applet;

/** Tests returning null values of all boxing types as well as String and Object */

public class NullValueTest extends Applet {
    public Boolean   getNullBoolean()   { return null; }
    public Byte      getNullByte()      { return null; }
    public Character getNullCharacter() { return null; }
    public Short     getNullShort()     { return null; }
    public Integer   getNullInteger()   { return null; }
    public Long      getNullLong()      { return null; }
    public Float     getNullFloat()     { return null; }
    public Double    getNullDouble()    { return null; }
    public String    getNullString()    { return null; }
    public Object    getNullObject()    { return null; }
}
