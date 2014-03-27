package arduinoupdater;

import arduinoupdater.UpdateWindow;
import javax.swing.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import java.net.*;

public class Updater implements ActionListener
{
    UpdateWindow win;
    static String base;
    static String os;

    public static void setConfiguration(String baseurl, String osname)
    {
        base=baseurl;
        os=osname;
    }

    public void run(String[] a) {
        win = new UpdateWindow(this);

        win.addWindowListener(
                              new WindowAdapter() {
                                  public void windowClosing( WindowEvent e )
                                  {
                                      System.exit( 0 );
                                  }
                              });
        //update();
    }

    public static File getBaseDirectory()
    {
        String baseDir = System.getProperty("user.dir");
        System.out.println(baseDir);
        return new File(baseDir);
    }

    public static File getBaseFile(String name) {

        return new File(getBaseDirectory(), name);
    }

    static void validateInstallation(List<String> path, File root, Map<String,String> sums)
    {
        File[] files = root.listFiles();
        for (File f: files) {
            if (f.isDirectory()) {
                path.add(f.getName());
                validateInstallation(path, f,sums);
                path.remove(path.size()-1);
            } else {
                for (String i: path) {
                    System.out.print(i + " ");
                }
                System.out.println(f.getName());
                try {
                    MessageDigest md = MessageDigest.getInstance("SHA-1");
                    FileInputStream fis = new FileInputStream(f);
                    byte[] data = new byte[1024];
                    int read = 0;
                    while ((read = fis.read(data)) != -1) {
                        md.update(data, 0, read);
                    };
                    byte[] hashBytes = md.digest();
                    String fname;
                    if (path.size()>0) {
                        fname = join(path,"/")+"/"+f.getName();
                    } else {
                        fname = f.getName();
                    }
                    sums.put(fname,getHashString(hashBytes));
                } catch (Exception e) {
                    System.out.println("Cannot compute!");
                }
            }
        }
    }

    static public String join(List<String> list, String conjunction)
    {
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for (String item : list)
        {
            if (first)
                first = false;
            else
                sb.append(conjunction);
            sb.append(item);
        }
        return sb.toString();
    }
    static String getHashString(byte[] hashBytes) {
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < hashBytes.length; i++) {
            sb.append(Integer.toString((hashBytes[i] & 0xff) + 0x100, 16).substring(1));
        }
        return sb.toString();
    }

    public static String getBaseHREF() {

        return new String( base + "/" + os + "/" + "updatelist.xml");
    }

    public static Document fetchUpdateData(boolean compressed) {
        URL base;

        InputStream stream;
        GZIPInputStream gzstream=null;

        try {
            if (compressed) {
                base = new URL(getBaseHREF()+".gz");
            } else {
                base = new URL(getBaseHREF());
            }
            URLConnection conn = base.openConnection();
            conn.setUseCaches(false);

            stream = conn.getInputStream();
            if (compressed) {
                gzstream = new GZIPInputStream(stream);
            }

            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc;

            if (compressed) {
                doc = dBuilder.parse(gzstream);
            } else {
                doc = dBuilder.parse(stream);
            }
            return doc;
        } catch (MalformedURLException mfue) {
            // not a url, that's fine
            mfue.printStackTrace();
            return null;
        } catch (FileNotFoundException fnfe) {
            // Java 1.5 likes to throw this when URL not available. (fix for 0119)
            // http://dev.processing.org/bugs/show_bug.cgi?id=403
            fnfe.printStackTrace();
            return null;
        } catch (IOException e) {
            // changed for 0117, shouldn't be throwing exception
            e.printStackTrace();
            //System.err.println("Error downloading from URL " + filename);
            return null;
            //throw new RuntimeException("Error downloading from URL " + filename);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    int step = 0;

    void nextStep()
    {
        switch (step) {
        case 0:
            win.displayMessage("Please wait while your installation is validated...");
            win.disableAllButtons();
            initialValidation();
            win.enableAllButtons();
            win.displayMessage("Installation validated. Click next to contact update server.");
            step++;
            break;
        case 1:
            win.disableAllButtons();
            win.displayMessage("Contacting the update server...");
            fetch();
            if (data==null) {
                win.displayMessage("Cannot contact update server.");
            } else {
                win.displayMessage("Update server contacted. Got "+data.getNumberOfReleases() +
                                   " releases across " + data.getNumberOfBranches() + " branches.");
                win.enableAllButtons();
            }

        }
    }


    public void actionPerformed(ActionEvent e) {
        String command = e.getActionCommand();
        if ("cancel".equals(command)) {
        }
        else if ("next".equals(command)) {
            nextStep();
        }
        else if ("next".equals(command)) {
        }

    }

    public void configure(Document doc) throws MalformedXMLException
    {
        /// ....

        Element root = doc.getDocumentElement();
        if (root.getTagName() != "Updater")
            throw new MalformedXMLException();

        Element baseHREFnode = XMLUtil.getFirstChild(root, "BaseHREF");
        Element archNode = XMLUtil.getFirstChild(root, "Arch");

        setConfiguration(XMLUtil.getText(baseHREFnode),
                         XMLUtil.getText(archNode));
    }

    public void initialValidation()
    {
        File configuration = getBaseFile("updater.xml");
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();

        Map<String,String> hashes = new HashMap<String,String>();

        try {
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc = dBuilder.parse(configuration);

            // Handle doc tags. Need those to configure the updater

            configure(doc);

            List<String> path = new ArrayList<String>();
            validateInstallation( path, getBaseDirectory(), hashes );

            for (Map.Entry<String, String> entry : hashes.entrySet())
            {
                System.out.println(entry.getKey() + ":" + entry.getValue());
            }

        } catch (Exception e) {
            System.out.println("Cannot load configuration file");
        }
    }
    ReleaseData data;

    void fetch()
    {
        Document filelist = fetchUpdateData(true);
        try {
            data = new ReleaseData();
            data.parse(filelist);

            /*
            ReleaseData.Release r = d.findReleaseByVersion("1.0.1");

            ReleaseData.MatchResult res = d.compareToRelease(r, hashes);

            Debugger.debug("New files: " + res.newFiles.size());
            Debugger.debug("Deprecated files: " + res.deprecatedFiles.size());
            Debugger.debug("Replaced files: " + res.replacedFiles.size());
            Debugger.debug("Kept files: " + res.keptFiles.size());

            data.downloadRelease(res.newFiles, r.basedir);
            data.downloadRelease(res.replacedFiles, r.basedir);

            data.applyDownloads(res);*/
        } catch (MalformedXMLException e) {
            e.printStackTrace();
            System.err.println("Cannot parse file");
            data=null;
//        } catch (ReleaseData.DownloadError e) {
//            e.printStackTrace();
//        } catch (IOException e) {
  //          e.printStackTrace();
   //         data=null;
        } catch (Exception e) {
            e.printStackTrace();
            data=null;
        }
    }
};
