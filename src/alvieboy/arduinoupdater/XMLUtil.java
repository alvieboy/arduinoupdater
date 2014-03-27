package arduinoupdater;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;

class XMLUtil
{
    static Element getFirstChild(Element base, String name)
    {
        for(Node n=base.getFirstChild(); n!=null; n=n.getNextSibling()){
            if (n.getNodeType()==Node.ELEMENT_NODE) {
                if (((Element)n).getTagName().equals(name))
                    return (Element)n;
            }
        }
        return null;
    }

    static String getText(Element e) {
        return e.getChildNodes().item(0).getNodeValue();
    }
}